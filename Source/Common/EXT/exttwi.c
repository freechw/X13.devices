/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Extensions TWI/I2C

#include "../config.h"

#ifdef EXTTWI_USED

#define TWIM_BUS_TIMEOUT            250 // ms

// Global variable used in HAL
volatile TWI_QUEUE_t  * pTWI = NULL;
static uint32_t twi_ms  = 0;

#ifndef EXTPLC_USED

// local queues
static Queue_t  twi_tx_queue = {NULL, NULL, 4, 0};      // Max Size = 4 records

static e_MQTTSN_RETURNS_t twiReadOD(subidx_t * pSubidx __attribute__ ((unused)),
                                        uint8_t *pLen, uint8_t *pBuf)
{
    if(pTWI == NULL)
        return MQTTSN_RET_REJ_CONG;

    *pLen = pTWI->frame.read + sizeof(TWI_FRAME_t);
    memcpy(pBuf, (void *)&pTWI->frame, *pLen);
    mqFree((void *)pTWI);
    pTWI = NULL;
    return MQTTSN_RET_ACCEPTED;
}

static e_MQTTSN_RETURNS_t twiWriteOD(subidx_t * pSubidx __attribute__ ((unused)),
                                        uint8_t Len, uint8_t *pBuf)
{
    if(Len < sizeof(TWI_FRAME_t))
        return MQTTSN_RET_REJ_NOT_SUPP;

    TWI_QUEUE_t * pQueue = mqAlloc(sizeof(MQ_t));

    memcpy(&pQueue->frame, pBuf, Len);
    
    pQueue->frame.access &= (TWI_WRITE | TWI_READ);
    pQueue->frame.write = (Len - sizeof(TWI_FRAME_t));
    if(pQueue->frame.write != 0)
    {
        pQueue->frame.access |= TWI_WRITE;
    }
    if(pQueue->frame.read != 0)
    {
        pQueue->frame.access |= TWI_READ;
    }

    if((pQueue->frame.read > (MQTTSN_MSG_SIZE - sizeof(TWI_FRAME_t) - MQTTSN_SIZEOF_MSG_PUBLISH)) ||
       ((pQueue->frame.access & (TWI_WRITE | TWI_READ)) == 0))
    {
        mqFree(pQueue);
        return MQTTSN_RET_REJ_NOT_SUPP;
    }

    if(!mqEnqueue(&twi_tx_queue, pQueue))
    {
        mqFree(pQueue);
        return MQTTSN_RET_REJ_CONG;
    }

    return MQTTSN_RET_ACCEPTED;
}

static uint8_t twiPollOD(subidx_t * pSubidx __attribute__ ((unused)))
{
    if(pTWI != NULL)
    {
        uint8_t access = pTWI->frame.access;
        if((access & (TWI_ERROR | TWI_SLANACK | TWI_WD)) != 0)      // Error state
        {
            return 1;
        }
        else if(access & TWI_RDY)
        {
            if(pTWI->frame.read != 0)
            {
                return 1;
            }
            else
            {
                mqFree((void *)pTWI);
                pTWI = NULL;
            }
        }
        else
        {
            if((HAL_get_ms() - twi_ms) > TWIM_BUS_TIMEOUT)
            {
                if(access & TWI_BUSY)
                {
                    hal_twi_stop();
                }
                
                pTWI->frame.access |= TWI_WD;
                return 1;
            }

            if((access & TWI_BUSY) == 0)
            {
                hal_twi_start();
            }
        }
    }
    else if(twi_tx_queue.Size != 0)
    {
        pTWI = mqDequeue(&twi_tx_queue);
        twi_ms = HAL_get_ms();
    }

    return 0;
}

#endif  //  EXTPLC_USED
void twiInit()
{
    uint8_t scl, sda;

    hal_twi_get_pins(&scl, &sda);
    dioTake(scl);
    dioTake(sda);

    if(!hal_twi_configure(1))           // Enable
    {
        dioRelease(scl);
        dioRelease(sda);
        return;
    }

    if(pTWI != NULL)
    {
        mqFree((void *)pTWI);
        pTWI = NULL;
    }

#ifndef EXTPLC_USED
    // Register variable Ta0
    indextable_t * pIndex = getFreeIdxOD();
    if(pIndex == NULL)
    {
        dioRelease(scl);
        dioRelease(sda);
        hal_twi_configure(0);
        return;
    }

    pIndex->cbRead     = &twiReadOD;
    pIndex->cbWrite    = &twiWriteOD;
    pIndex->cbPoll     = &twiPollOD;
    pIndex->sidx.Place = objTWI;        // TWI object
    pIndex->sidx.Type  = objArray;      // Variable Type -  Byte Array
    pIndex->sidx.Base  = 0;             // Device address
#endif  //  EXTPLC_USED
}

#ifdef EXTPLC_USED
static uint8_t  twi_pnt = 0;

void twiControl(uint32_t ctrl)
{
    if(pTWI != NULL)
        return;
    
    pTWI = mqAlloc(sizeof(MQ_t));
    if(pTWI == NULL)
        return;

    twi_pnt = 0;
    twi_ms = HAL_get_ms();
    
    memcpy((void *)&pTWI->frame, &ctrl, 4);

    pTWI->frame.access &= (TWI_WRITE | TWI_READ);

    if(pTWI->frame.read != 0)
        pTWI->frame.access |= TWI_READ;    

    if(pTWI->frame.write != 0)
        pTWI->frame.access |= TWI_WRITE;
    else
        hal_twi_start();
}

uint32_t twiStat(void)
{
    if(pTWI == NULL)
        return 0;

    uint8_t access = pTWI->frame.access;

    if((access & (TWI_WD | TWI_SLANACK | TWI_ERROR)) == 0)
    {
        if((HAL_get_ms() - twi_ms) > TWIM_BUS_TIMEOUT)
        {
            if(pTWI->frame.access & TWI_BUSY)
                hal_twi_stop();
            
            pTWI->frame.access |= TWI_WD;
            access |= TWI_WD;
        }
    }

    uint32_t retval;
    memcpy(&retval, (void *)&pTWI->frame, 4);
    
    if(access & (TWI_WD | TWI_SLANACK | TWI_ERROR))     // Error state
    {
        mqFree((void *)pTWI);
        pTWI = NULL;
    }
    else if(access & TWI_RDY)
    {
        twi_pnt = 0;
        if(pTWI->frame.read == 0)              // Bus Free
        {
            mqFree((void *)pTWI);
            pTWI = NULL;
        }
    }
    
    return retval;
}

void twiWr(uint8_t data)
{
    if( (pTWI == NULL) || 
       ((pTWI->frame.access & (TWI_BUSY | TWI_RDY | TWI_WD | TWI_SLANACK | TWI_ERROR)) != 0))
        return;
    
    if(twi_pnt < pTWI->frame.write)
    {
        pTWI->frame.data[twi_pnt++] = data;
        if(twi_pnt == pTWI->frame.write)
            hal_twi_start();
    }
}

uint8_t twiRd(void)
{
    uint8_t retval = 0;
    
    if((pTWI != NULL) && (pTWI->frame.access & TWI_RDY))
    {
        if(twi_pnt < pTWI->frame.read)
            retval = pTWI->frame.data[twi_pnt++];
        
        if(twi_pnt == pTWI->frame.read)
        {
            mqFree((void *)pTWI);
            pTWI = NULL;
        }
    }

    return retval;
}

#endif  //  EXTPLC_USED


#endif    //  EXTTWI_USED
