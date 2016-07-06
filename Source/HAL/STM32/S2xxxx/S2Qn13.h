/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _S2QN13_H
#define _S2QN13_H

// Board: S2ER13
// uC: STM32F051C8T6
// PHY1: RFM69
// PHY2: --

// GPIOA
// Pin  Port    CN  Func            PWM
//   0  PA0     3                   !TIM2_CH1
//   1  PA1     4                   TIM2_CH2
//   2  PA2     5   _USART2_TX      TIM15_CH1
//   3  PA3     6   _USART2_RX      TIM15_CH2
//   4  PA4     15                  TIM14_CH1
//   5  PA5     16                  !TIM2_CH1
//   6  PA6     17                  TIM3_CH1
//   7  PA7     18                  TIM3_CH2
//   8  PA8         RFM69_IRQ
//   9  PA9     9   _USART1_TX      TIM1_CH2
//  10  PA10    10  _USART1_RX      TIM1_CH3
//  11  PA11    7                   TIM1_CH4
//  12  PA12    8
//  13  PA13        SWDIO
//  14  PA14        SWCLK
//  15  PA15        L_IRQ
// GPIOB
//  16  PB0     19                  TIM3_CH3
//  17  PB1     20                  TIM3_CH4
//  18  PB2         LED
//  19  PB3         L_SCK
//  20  PB4         L_MISO
//  21  PB5         L_MOSI
//  22  PB6         L_SEL_TX1
//  23  PB7         L_RST_RX1
//  24  PB8
//  25  PB9
//  26  PB10    13  SCL2            TIM2_CH3
//  27  PB11    14  SDA2            TIM2_CH4
//  28  PB12        RFM69_SEL
//  29  PB13        RFM69_SCK
//  30  PB14        RFM69_MISO
//  31  PB15        RFM69_MOSI

#ifdef __cplusplus
extern "C" {
#endif

// System Settings
#define HAL_USE_RTC                 1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING             {17, 16, 7, 6, 5, 4, 27, 26, 10, 9, 12, 11, 3, 2, 1, 0}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {((1<<8) |  (3<<3) | 3),    /* PB1:  AF1, TIM3_CH4  */ \
                                     ((1<<8) |  (3<<3) | 2),    /* PB0:  AF1, TIM3_CH3  */ \
                                     ((1<<8) |  (3<<3) | 1),    /* PA7:  AF1, TIM3_CH2  */ \
                                     ((1<<8) |  (3<<3) | 0),    /* PA6:  AF1, TIM3_CH1  */ \
                                     ((2<<8) |  (2<<3) | 0),    /* PA5:  AF2, TIM2_CH1  */ \
                                     ((4<<8) | (14<<3) | 0),    /* PA4:  AF4, TIM14_CH1 */ \
                                     ((2<<8) |  (2<<3) | 3),    /* PB11: AF2, TIM2_CH4  */ \
                                     ((2<<8) |  (2<<3) | 2),    /* PB10: AF2, TIM2_CH3  */ \
                                     ((2<<8) |  (1<<3) | 2),    /* PA10: AF2, TIM1_CH3  */ \
                                     ((2<<8) | ( 1<<3) | 1),    /* PA9:  AF2, TIM1_CH2  */ \
                                     255,                       /* PA12: No Config      */ \
                                     ((2<<8) |  (1<<3) | 3),    /* PA11: AF2, TIM1_CH4  */ \
                                     ((0<<8) | (15<<3) | 1),    /* PA3:  AF0, TIM15_CH2 */ \
                                     ((0<<8) | (15<<3) | 0),    /* PA2:  AF0, TIM15_CH1 */ \
                                     ((2<<8) |  (2<<3) | 1),    /* PA1:  AF2, TIM2_CH2  */ \
                                     ((2<<8) |  (2<<3) | 0)}    /* PA0:  AF2, TIM2_CH1  */
// End PWM Section

// PA0-PA7: 0 - 7
// PB0-PB1: 8 - 9
// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           10
#define HAL_AIN_BASE2APIN           {9, 8, 7, 6, 5, 4, 255, 255, 255, 255, 255, 255, 3, 2, 1, 0}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART1              1           // Mapping to logical port
#define HAL_USE_USART2              0
#define EXTSER_USED                 2
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 2       // I2C_Bus 1 - I2C1, 2 - I2C2
#define EXTTWI_USED                 1
// End TWI Section

// LEDs
#define LED_On()                    GPIOB->BSRR = GPIO_BSRR_BS_2
#define LED_Off()                   GPIOB->BSRR = GPIO_BSRR_BR_2
#define LED_Init()                  hal_gpio_cfg(GPIOB, GPIO_Pin_2, DIO_MODE_OUT_PP)

// RFM69 Section
#define HAL_USE_SPI2                1
#define RFM69_USE_SPI               2
#define RFM69_NSS_PIN               28                          // PB12
#define RFM69_SELECT()              GPIOB->BRR = GPIO_Pin_12
#define RFM69_RELEASE()             GPIOB->BSRR = GPIO_Pin_12
#define RFM69_IRQ_PIN               8                           // PA8
#define RFM69_IRQ_STATE()           ((GPIOA->IDR & GPIO_Pin_8) != 0)
#define RFM69_PHY                   1
#include "PHY/RFM69/rfm69_phy.h"
// End RFM69 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           20
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'Q'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '3'

#ifdef __cplusplus
}
#endif

#endif // _S2QN13_H
