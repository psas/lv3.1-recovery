/*
    ChibiOS - Copyright (C) 2006..2020 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
 * This file has been automatically generated using ChibiStudio board
 * generator plugin. Do not edit manually.
 */

#ifndef BOARD_H
#define BOARD_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*
 * Setup for STMicroelectronics STM32 Nucleo32-F042K6 board.
 */

/*
 * Board identifier.
 */
#define BOARD_ST_NUCLEO32_F042K6
#define BOARD_NAME                  "STMicroelectronics STM32 Nucleo32-F042K6"

/*
 * Board oscillators-related settings.
 * NOTE: LSE not fitted.
 * NOTE: HSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0U
#endif

#define STM32_LSEDRV                (3U << 3U)

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                0U
#endif

/*
 * MCU type as defined in the ST header.
 */
#define STM32F042x6

/*
 * IO pins assignments.
 */
#define GPIO_PA0_DCM_SPEED        0U
#define GPIO_PA1_LA_POS           1U
#define GPIO_PA2_UART2_TX         2U
#define GPIO_PA3_ISO_MAIN         3U
#define GPIO_PA4_ISO_DROGUE       4U
#define GPIO_PA5_DCM_PWM          5U
#define GPIO_PA6_DCM_DIR          6U
#define GPIO_PA7_DCM_PWR          7U
#define GPIO_PA8_LA_DRV1          8U
#define GPIO_PA9_LA_DRV2          9U
#define GPIO_PA10_NC              10U
#define GPIO_PA11_NC              11U
#define GPIO_PA12_NC              12U
#define GPIO_PA13_SWDIO           13U
#define GPIO_PA14_SWCLK           14U
#define GPIO_PA15_UART2_RX        15U

#define GPIO_PB0_LED              0U
#define GPIO_PB1_BATT_READ        1U
#define GPIO_PB2_NOPIN            2U
#define GPIO_PB3_ROTSENSE_PWR     3U
#define GPIO_PB4_ROTSENSE2        4U
#define GPIO_PB5_ROTSENSE1        5U
#define GPIO_PB6_NC               6U
#define GPIO_PB7_UMB_ON           7U
#define GPIO_PB8_SPKR             8U
#define GPIO_PB9_NOPIN            9U
#define GPIO_PB10_NOPIN           10U
#define GPIO_PB11_NOPIN           11U
#define GPIO_PB12_NOPIN           12U
#define GPIO_PB13_NOPIN           13U
#define GPIO_PB14_NOPIN           14U
#define GPIO_PB15_NOPIN           15U

#define GPIO_PC0_NOPIN            0U
#define GPIO_PC1_NOPIN            1U
#define GPIO_PC2_NOPIN            2U
#define GPIO_PC3_NOPIN            3U
#define GPIO_PC4_NOPIN            4U
#define GPIO_PC5_NOPIN            5U
#define GPIO_PC6_NOPIN            6U
#define GPIO_PC7_NOPIN            7U
#define GPIO_PC8_NOPIN            8U
#define GPIO_PC9_NOPIN            9U
#define GPIO_PC10_NOPIN           10U
#define GPIO_PC11_NOPIN           11U
#define GPIO_PC12_NOPIN           12U
#define GPIO_PC13_NOPIN           13U
#define GPIO_PC14_NOPIN           14U
#define GPIO_PC15_NOPIN           15U

#define GPIO_PD0_NOPIN            0U
#define GPIO_PD1_NOPIN            1U
#define GPIO_PD2_NOPIN            2U
#define GPIO_PD3_NOPIN            3U
#define GPIO_PD4_NOPIN            4U
#define GPIO_PD5_NOPIN            5U
#define GPIO_PD6_NOPIN            6U
#define GPIO_PD7_NOPIN            7U
#define GPIO_PD8_NOPIN            8U
#define GPIO_PD9_NOPIN            9U
#define GPIO_PD10_NOPIN           10U
#define GPIO_PD11_NOPIN           11U
#define GPIO_PD12_NOPIN           12U
#define GPIO_PD13_NOPIN           13U
#define GPIO_PD14_NOPIN           14U
#define GPIO_PD15_NOPIN           15U

#define GPIO_PE0_NOPIN            0U
#define GPIO_PE1_NOPIN            1U
#define GPIO_PE2_NOPIN            2U
#define GPIO_PE3_NOPIN            3U
#define GPIO_PE4_NOPIN            4U
#define GPIO_PE5_NOPIN            5U
#define GPIO_PE6_NOPIN            6U
#define GPIO_PE7_NOPIN            7U
#define GPIO_PE8_NOPIN            8U
#define GPIO_PE9_NOPIN            9U
#define GPIO_PE10_NOPIN           10U
#define GPIO_PE11_NOPIN           11U
#define GPIO_PE12_NOPIN           12U
#define GPIO_PE13_NOPIN           13U
#define GPIO_PE14_NOPIN           14U
#define GPIO_PE15_NOPIN           15U

#define GPIO_PF0_OSC_IN           0U
#define GPIO_PF1_OSC_OUT          1U
#define GPIO_PF2_NOPIN            2U
#define GPIO_PF3_NOPIN            3U
#define GPIO_PF4_NOPIN            4U
#define GPIO_PF5_NOPIN            5U
#define GPIO_PF6_NOPIN            6U
#define GPIO_PF7_NOPIN            7U
#define GPIO_PF8_NOPIN            8U
#define GPIO_PF9_NOPIN            9U
#define GPIO_PF10_NOPIN           10U
#define GPIO_PF11_NOPIN           11U
#define GPIO_PF12_NOPIN           12U
#define GPIO_PF13_NOPIN           13U
#define GPIO_PF14_NOPIN           14U
#define GPIO_PF15_NOPIN           15U

/*
 * IO lines assignments.
 */

#define LINE_DCM_SPEED                PAL_LINE(GPIOA, 0U)
#define LINE_LA_POS                   PAL_LINE(GPIOA, 1U)
#define LINE_UART2_TX                 PAL_LINE(GPIOA, 2U)
#define LINE_ISO_MAIN                 PAL_LINE(GPIOA, 3U)
#define LINE_ISO_DROGUE               PAL_LINE(GPIOA, 4U)
#define LINE_DCM_PWM                  PAL_LINE(GPIOA, 5U)
#define LINE_DCM_DIR                  PAL_LINE(GPIOA, 6U)
#define LINE_DCM_PWR                  PAL_LINE(GPIOA, 7U)
#define LINE_LA_DRV1                  PAL_LINE(GPIOA, 8U)
#define LINE_LA_DRV2                  PAL_LINE(GPIOA, 9U)
#define LINE_SWDIO                    PAL_LINE(GPIOA, 13U)
#define LINE_SWCLK                    PAL_LINE(GPIOA, 14U)
#define LINE_UART2_RX                 PAL_LINE(GPIOA, 15U)
#define LINE_LED                      PAL_LINE(GPIOB, 0U)
#define LINE_BATT_READ                PAL_LINE(GPIOB, 1U)
#define LINE_ROTSENSE_PWR             PAL_LINE(GPIOB, 3U)
#define LINE_ROTSENSE2                PAL_LINE(GPIOB, 4U)
#define LINE_ROTSENSE1                PAL_LINE(GPIOB, 5U)
#define LINE_UMB_ON                   PAL_LINE(GPIOB, 7U)
#define LINE_SPKR                     PAL_LINE(GPIOB, 8U)
#define LINE_OSC_IN                   PAL_LINE(GPIOF, 0U)
#define LINE_OSC_OUT                  PAL_LINE(GPIOF, 1U)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2U))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2U))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2U))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2U))

#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))

#define PIN_OSPEED_VERYLOW(n)       (0U << ((n) * 2U))
#define PIN_OSPEED_LOW(n)           (1U << ((n) * 2U))
#define PIN_OSPEED_MEDIUM(n)        (2U << ((n) * 2U))
#define PIN_OSPEED_HIGH(n)          (3U << ((n) * 2U))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2U))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2U))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2U))
#define PIN_AFIO_AF(n, v)           ((v) << (((n) % 8U) * 4U))


/*
 * 
 * GPIOA setup
 *
 */

#define VAL_GPIOA_MODER   (PIN_MODE_INPUT     (GPIO_PA0_DCM_SPEED) | \
                           PIN_MODE_ANALOG    (GPIO_PA1_LA_POS)    | \
                           PIN_MODE_ALTERNATE (GPIO_PA2_UART2_TX)  | \
                           PIN_MODE_INPUT     (GPIO_PA3_ISO_MAIN)  | \
                           PIN_MODE_INPUT     (GPIO_PA4_ISO_DROGUE)| \
                           PIN_MODE_ALTERNATE (GPIO_PA5_DCM_PWM)   | \
                           PIN_MODE_OUTPUT    (GPIO_PA6_DCM_DIR)   | \
                           PIN_MODE_OUTPUT    (GPIO_PA7_DCM_PWR)   | \
                           PIN_MODE_OUTPUT    (GPIO_PA8_LA_DRV1)   | \
                           PIN_MODE_OUTPUT    (GPIO_PA9_LA_DRV2)   | \
                           PIN_MODE_INPUT     (GPIO_PA10_NC)       | \
                           PIN_MODE_INPUT     (GPIO_PA11_NC)       | \
                           PIN_MODE_INPUT     (GPIO_PA12_NC)       | \
                           PIN_MODE_ALTERNATE (GPIO_PA13_SWDIO)    | \
                           PIN_MODE_ALTERNATE (GPIO_PA14_SWCLK)    | \
                           PIN_MODE_ALTERNATE (GPIO_PA15_UART2_RX))

#define VAL_GPIOA_OTYPER  (PIN_OTYPE_PUSHPULL (GPIO_PA0_DCM_SPEED) | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA1_LA_POS)    | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA2_UART2_TX)  | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA3_ISO_MAIN)  | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA4_ISO_DROGUE)| \
                           PIN_OTYPE_PUSHPULL (GPIO_PA5_DCM_PWM)   | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA6_DCM_DIR)   | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA7_DCM_PWR)   | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA8_LA_DRV1)   | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA9_LA_DRV2)   | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA10_NC)       | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA11_NC)       | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA12_NC)       | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA13_SWDIO)    | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA14_SWCLK)    | \
                           PIN_OTYPE_PUSHPULL (GPIO_PA15_UART2_RX))

#define VAL_GPIOA_OSPEEDR (PIN_OSPEED_LOW  (GPIO_PA0_DCM_SPEED) | \
                           PIN_OSPEED_LOW  (GPIO_PA1_LA_POS)    | \
                           PIN_OSPEED_LOW  (GPIO_PA2_UART2_TX)  | \
                           PIN_OSPEED_LOW  (GPIO_PA3_ISO_MAIN)  | \
                           PIN_OSPEED_LOW  (GPIO_PA4_ISO_DROGUE)| \
                           PIN_OSPEED_LOW  (GPIO_PA5_DCM_PWM)   | \
                           PIN_OSPEED_LOW  (GPIO_PA6_DCM_DIR)   | \
                           PIN_OSPEED_LOW  (GPIO_PA7_DCM_PWR)   | \
                           PIN_OSPEED_LOW  (GPIO_PA8_LA_DRV1)   | \
                           PIN_OSPEED_LOW  (GPIO_PA9_LA_DRV2)   | \
                           PIN_OSPEED_LOW  (GPIO_PA10_NC)       | \
                           PIN_OSPEED_LOW  (GPIO_PA11_NC)       | \
                           PIN_OSPEED_LOW  (GPIO_PA12_NC)       | \
                           PIN_OSPEED_HIGH (GPIO_PA13_SWDIO)    | \
                           PIN_OSPEED_HIGH (GPIO_PA14_SWCLK)    | \
                           PIN_OSPEED_LOW  (GPIO_PA15_UART2_RX))

#define VAL_GPIOA_PUPDR   (PIN_PUPDR_PULLUP   (GPIO_PA0_DCM_SPEED) | \
                           PIN_PUPDR_FLOATING (GPIO_PA1_LA_POS)    | \
                           PIN_PUPDR_FLOATING (GPIO_PA2_UART2_TX)  | \
                           PIN_PUPDR_FLOATING (GPIO_PA3_ISO_MAIN)  | \
                           PIN_PUPDR_FLOATING (GPIO_PA4_ISO_DROGUE)| \
                           PIN_PUPDR_FLOATING (GPIO_PA5_DCM_PWM)   | \
                           PIN_PUPDR_FLOATING (GPIO_PA6_DCM_DIR)   | \
                           PIN_PUPDR_FLOATING (GPIO_PA7_DCM_PWR)   | \
                           PIN_PUPDR_FLOATING (GPIO_PA8_LA_DRV1)   | \
                           PIN_PUPDR_FLOATING (GPIO_PA9_LA_DRV2)   | \
                           PIN_PUPDR_PULLDOWN (GPIO_PA10_NC)       | \
                           PIN_PUPDR_PULLDOWN (GPIO_PA11_NC)       | \
                           PIN_PUPDR_PULLDOWN (GPIO_PA12_NC)       | \
                           PIN_PUPDR_PULLUP   (GPIO_PA13_SWDIO)    | \
                           PIN_PUPDR_PULLDOWN (GPIO_PA14_SWCLK)    | \
                           PIN_PUPDR_PULLUP   (GPIO_PA15_UART2_RX))

#define VAL_GPIOA_ODR     (PIN_ODR_LOW  (GPIO_PA0_DCM_SPEED) | \
                           PIN_ODR_LOW  (GPIO_PA1_LA_POS)    | \
                           PIN_ODR_HIGH (GPIO_PA2_UART2_TX)  | \
                           PIN_ODR_LOW  (GPIO_PA3_ISO_MAIN)  | \
                           PIN_ODR_LOW  (GPIO_PA4_ISO_DROGUE)| \
                           PIN_ODR_LOW  (GPIO_PA5_DCM_PWM)   | \
                           PIN_ODR_LOW  (GPIO_PA6_DCM_DIR)   | \
                           PIN_ODR_LOW  (GPIO_PA7_DCM_PWR)   | \
                           PIN_ODR_LOW  (GPIO_PA8_LA_DRV1)   | \
                           PIN_ODR_LOW  (GPIO_PA9_LA_DRV2)   | \
                           PIN_ODR_LOW  (GPIO_PA10_NC)       | \
                           PIN_ODR_LOW  (GPIO_PA11_NC)       | \
                           PIN_ODR_LOW  (GPIO_PA12_NC)       | \
                           PIN_ODR_HIGH (GPIO_PA13_SWDIO)    | \
                           PIN_ODR_LOW  (GPIO_PA14_SWCLK)    | \
                           PIN_ODR_HIGH (GPIO_PA15_UART2_RX))

#define VAL_GPIOA_AFRL    (PIN_AFIO_AF (GPIO_PA0_DCM_SPEED,  0U) | \
                           PIN_AFIO_AF (GPIO_PA1_LA_POS,     0U) | \
                           PIN_AFIO_AF (GPIO_PA2_UART2_TX,   1U) | \
                           PIN_AFIO_AF (GPIO_PA3_ISO_MAIN,   0U) | \
                           PIN_AFIO_AF (GPIO_PA4_ISO_DROGUE, 0U) | \
                           PIN_AFIO_AF (GPIO_PA5_DCM_PWM,    1U) | \
                           PIN_AFIO_AF (GPIO_PA6_DCM_DIR,    0U) | \
                           PIN_AFIO_AF (GPIO_PA7_DCM_PWR,    0U))

#define VAL_GPIOA_AFRH    (PIN_AFIO_AF (GPIO_PA8_LA_DRV1,   0U) | \
                           PIN_AFIO_AF (GPIO_PA9_LA_DRV2,   0U) | \
                           PIN_AFIO_AF (GPIO_PA10_NC,       0U) | \
                           PIN_AFIO_AF (GPIO_PA11_NC,       0U) | \
                           PIN_AFIO_AF (GPIO_PA12_NC,       0U) | \
                           PIN_AFIO_AF (GPIO_PA13_SWDIO,    1U) | \
                           PIN_AFIO_AF (GPIO_PA14_SWCLK,    1U) | \
                           PIN_AFIO_AF (GPIO_PA15_UART2_RX, 1U))

/*
 * 
 * GPIOB setup
 *
 */


#define VAL_GPIOB_MODER             (PIN_MODE_OUTPUT (GPIO_PB0_LED)          | \
                                     PIN_MODE_ANALOG (GPIO_PB1_BATT_READ)    | \
                                     PIN_MODE_INPUT  (GPIO_PB2_NOPIN)        | \
                                     PIN_MODE_OUTPUT (GPIO_PB3_ROTSENSE_PWR) | \
                                     PIN_MODE_INPUT  (GPIO_PB4_ROTSENSE2)    | \
                                     PIN_MODE_INPUT  (GPIO_PB5_ROTSENSE1)    | \
                                     PIN_MODE_INPUT  (GPIO_PB6_NC)           | \
                                     PIN_MODE_INPUT  (GPIO_PB7_UMB_ON)       | \
                                     PIN_MODE_OUTPUT (GPIO_PB8_SPKR)         | \
                                     PIN_MODE_INPUT  (GPIO_PB9_NOPIN)        | \
                                     PIN_MODE_INPUT  (GPIO_PB10_NOPIN)       | \
                                     PIN_MODE_INPUT  (GPIO_PB11_NOPIN)       | \
                                     PIN_MODE_INPUT  (GPIO_PB12_NOPIN)       | \
                                     PIN_MODE_INPUT  (GPIO_PB13_NOPIN)       | \
                                     PIN_MODE_INPUT  (GPIO_PB14_NOPIN)       | \
                                     PIN_MODE_INPUT  (GPIO_PB15_NOPIN))

#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL (GPIO_PB0_LED)          | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB1_BATT_READ)    | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB2_NOPIN)        | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB3_ROTSENSE_PWR) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB4_ROTSENSE2)    | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB5_ROTSENSE1)    | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB6_NC)           | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB7_UMB_ON)       | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB8_SPKR)         | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB9_NOPIN)        | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB10_NOPIN)       | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB11_NOPIN)       | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB12_NOPIN)       | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB13_NOPIN)       | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB14_NOPIN)       | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PB15_NOPIN))

#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_LOW  (GPIO_PB0_LED)          | \
                                     PIN_OSPEED_LOW  (GPIO_PB1_BATT_READ)    | \
                                     PIN_OSPEED_LOW  (GPIO_PB2_NOPIN)        | \
                                     PIN_OSPEED_LOW  (GPIO_PB3_ROTSENSE_PWR) | \
                                     PIN_OSPEED_LOW  (GPIO_PB4_ROTSENSE2)    | \
                                     PIN_OSPEED_LOW  (GPIO_PB5_ROTSENSE1)    | \
                                     PIN_OSPEED_LOW  (GPIO_PB6_NC)           | \
                                     PIN_OSPEED_LOW  (GPIO_PB7_UMB_ON)       | \
                                     PIN_OSPEED_LOW  (GPIO_PB8_SPKR)         | \
                                     PIN_OSPEED_LOW  (GPIO_PB9_NOPIN)        | \
                                     PIN_OSPEED_LOW  (GPIO_PB10_NOPIN)       | \
                                     PIN_OSPEED_LOW  (GPIO_PB11_NOPIN)       | \
                                     PIN_OSPEED_LOW  (GPIO_PB12_NOPIN)       | \
                                     PIN_OSPEED_LOW  (GPIO_PB13_NOPIN)       | \
                                     PIN_OSPEED_LOW  (GPIO_PB14_NOPIN)       | \
                                     PIN_OSPEED_LOW  (GPIO_PB15_NOPIN))

#define VAL_GPIOB_PUPDR             (PIN_PUPDR_FLOATING (GPIO_PB0_LED)          | \
                                     PIN_PUPDR_FLOATING (GPIO_PB1_BATT_READ)    | \
                                     PIN_PUPDR_PULLDOWN (GPIO_PB2_NOPIN)        | \
                                     PIN_PUPDR_FLOATING (GPIO_PB3_ROTSENSE_PWR) | \
                                     PIN_PUPDR_FLOATING (GPIO_PB4_ROTSENSE2)    | \
                                     PIN_PUPDR_FLOATING (GPIO_PB5_ROTSENSE1)    | \
                                     PIN_PUPDR_PULLDOWN (GPIO_PB6_NC)           | \
                                     PIN_PUPDR_FLOATING (GPIO_PB7_UMB_ON)       | \
                                     PIN_PUPDR_FLOATING (GPIO_PB8_SPKR)         | \
                                     PIN_PUPDR_PULLDOWN (GPIO_PB9_NOPIN)        | \
                                     PIN_PUPDR_PULLDOWN (GPIO_PB10_NOPIN)       | \
                                     PIN_PUPDR_PULLDOWN (GPIO_PB11_NOPIN)       | \
                                     PIN_PUPDR_PULLDOWN (GPIO_PB12_NOPIN)       | \
                                     PIN_PUPDR_PULLDOWN (GPIO_PB13_NOPIN)       | \
                                     PIN_PUPDR_PULLDOWN (GPIO_PB14_NOPIN)       | \
                                     PIN_PUPDR_PULLDOWN (GPIO_PB15_NOPIN))

#define VAL_GPIOB_ODR               (PIN_ODR_LOW  (GPIO_PB0_LED)          | \
                                     PIN_ODR_LOW  (GPIO_PB1_BATT_READ)    | \
                                     PIN_ODR_LOW  (GPIO_PB2_NOPIN)        | \
                                     PIN_ODR_LOW  (GPIO_PB3_ROTSENSE_PWR) | \
                                     PIN_ODR_LOW  (GPIO_PB4_ROTSENSE2)    | \
                                     PIN_ODR_LOW  (GPIO_PB5_ROTSENSE1)    | \
                                     PIN_ODR_LOW  (GPIO_PB6_NC)           | \
                                     PIN_ODR_LOW  (GPIO_PB7_UMB_ON)       | \
                                     PIN_ODR_LOW  (GPIO_PB8_SPKR)         | \
                                     PIN_ODR_LOW  (GPIO_PB9_NOPIN)        | \
                                     PIN_ODR_LOW  (GPIO_PB10_NOPIN)       | \
                                     PIN_ODR_LOW  (GPIO_PB11_NOPIN)       | \
                                     PIN_ODR_LOW  (GPIO_PB12_NOPIN)       | \
                                     PIN_ODR_LOW  (GPIO_PB13_NOPIN)       | \
                                     PIN_ODR_LOW  (GPIO_PB14_NOPIN)       | \
                                     PIN_ODR_LOW  (GPIO_PB15_NOPIN))

#define VAL_GPIOB_AFRL              (PIN_AFIO_AF (GPIO_PB0_LED,          0U) | \
                                     PIN_AFIO_AF (GPIO_PB1_BATT_READ,    0U) | \
                                     PIN_AFIO_AF (GPIO_PB2_NOPIN,        0U) | \
                                     PIN_AFIO_AF (GPIO_PB3_ROTSENSE_PWR, 0U) | \
                                     PIN_AFIO_AF (GPIO_PB4_ROTSENSE2,    0U) | \
                                     PIN_AFIO_AF (GPIO_PB5_ROTSENSE1,    0U) | \
                                     PIN_AFIO_AF (GPIO_PB6_NC,           0U) | \
                                     PIN_AFIO_AF (GPIO_PB7_UMB_ON,       0U))

#define VAL_GPIOB_AFRH              (PIN_AFIO_AF (GPIO_PB8_SPKR,   0U) | \
                                     PIN_AFIO_AF (GPIO_PB9_NOPIN,  0U) | \
                                     PIN_AFIO_AF (GPIO_PB10_NOPIN, 0U) | \
                                     PIN_AFIO_AF (GPIO_PB11_NOPIN, 0U) | \
                                     PIN_AFIO_AF (GPIO_PB12_NOPIN, 0U) | \
                                     PIN_AFIO_AF (GPIO_PB13_NOPIN, 0U) | \
                                     PIN_AFIO_AF (GPIO_PB14_NOPIN, 0U) | \
                                     PIN_AFIO_AF (GPIO_PB15_NOPIN, 0U))

/*
 * 
 * GPIOC setup
 *
 */

#define VAL_GPIOC_MODER             (PIN_MODE_INPUT (GPIO_PC0_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC1_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC2_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC3_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC4_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC5_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC6_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC7_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC8_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC9_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PC10_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PC11_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PC12_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PC13_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PC14_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PC15_NOPIN))

#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL (GPIO_PC0_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC1_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC2_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC3_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC4_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC5_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC6_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC7_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC8_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC9_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC10_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC11_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC12_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC13_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC14_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PC15_NOPIN))

#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_LOW  (GPIO_PC0_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC1_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC2_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC3_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC4_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC5_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC6_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC7_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC8_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC9_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PC10_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PC11_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PC12_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PC13_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PC14_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PC15_NOPIN))

#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP (GPIO_PC0_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC1_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC2_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC3_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC4_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC5_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC6_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC7_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC8_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC9_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PC10_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PC11_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PC12_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PC13_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PC14_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PC15_NOPIN))

#define VAL_GPIOC_ODR               (PIN_ODR_HIGH (GPIO_PC0_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC1_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC2_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC3_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC4_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC5_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC6_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC7_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC8_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC9_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PC10_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PC11_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PC12_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PC13_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PC14_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PC15_NOPIN))

#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIO_PC0_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC1_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC2_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC3_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC4_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC5_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC6_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC7_NOPIN, 0U))

#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIO_PC8_NOPIN,  0U) | \
                                     PIN_AFIO_AF(GPIO_PC9_NOPIN,  0U) | \
                                     PIN_AFIO_AF(GPIO_PC10_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC11_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC12_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC13_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC14_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PC15_NOPIN, 0U))

/*
 * 
 * GPIOD setup
 *
 */

#define VAL_GPIOD_MODER             (PIN_MODE_INPUT (GPIO_PD0_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD1_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD2_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD3_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD4_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD5_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD6_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD7_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD8_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD9_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PD10_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PD11_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PD12_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PD13_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PD14_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PD15_NOPIN))

#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL (GPIO_PD0_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD1_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD2_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD3_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD4_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD5_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD6_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD7_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD8_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD9_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD10_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD11_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD12_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD13_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD14_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PD15_NOPIN))

#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_LOW  (GPIO_PD0_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD1_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD2_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD3_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD4_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD5_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD6_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD7_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD8_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD9_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PD10_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PD11_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PD12_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PD13_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PD14_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PD15_NOPIN))

#define VAL_GPIOD_PUPDR             (PIN_PUPDR_PULLUP (GPIO_PD0_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD1_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD2_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD3_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD4_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD5_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD6_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD7_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD8_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD9_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PD10_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PD11_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PD12_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PD13_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PD14_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PD15_NOPIN))

#define VAL_GPIOD_ODR               (PIN_ODR_HIGH (GPIO_PD0_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD1_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD2_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD3_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD4_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD5_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD6_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD7_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD8_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD9_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PD10_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PD11_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PD12_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PD13_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PD14_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PD15_NOPIN))

#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIO_PD0_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD1_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD2_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD3_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD4_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD5_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD6_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD7_NOPIN, 0U))

#define VAL_GPIOD_AFRH              (PIN_AFIO_AF(GPIO_PD8_NOPIN,  0U) | \
                                     PIN_AFIO_AF(GPIO_PD9_NOPIN,  0U) | \
                                     PIN_AFIO_AF(GPIO_PD10_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD11_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD12_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD13_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD14_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PD15_NOPIN, 0U))


/*
 * 
 * GPIOE setup
 *
 */

#define VAL_GPIOE_MODER             (PIN_MODE_INPUT (GPIO_PE0_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE1_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE2_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE3_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE4_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE5_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE6_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE7_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE8_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE9_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PE10_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PE11_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PE12_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PE13_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PE14_NOPIN) | \
                                     PIN_MODE_INPUT (GPIO_PE15_NOPIN))

#define VAL_GPIOE_OTYPER            (PIN_OTYPE_PUSHPULL (GPIO_PE0_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE1_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE2_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE3_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE4_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE5_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE6_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE7_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE8_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE9_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE10_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE11_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE12_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE13_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE14_NOPIN) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PE15_NOPIN))

#define VAL_GPIOE_OSPEEDR           (PIN_OSPEED_LOW  (GPIO_PE0_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE1_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE2_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE3_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE4_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE5_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE6_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE7_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE8_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE9_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PE10_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PE11_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PE12_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PE13_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PE14_NOPIN) | \
                                     PIN_OSPEED_LOW  (GPIO_PE15_NOPIN))

#define VAL_GPIOE_PUPDR             (PIN_PUPDR_PULLUP (GPIO_PE0_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE1_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE2_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE3_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE4_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE5_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE6_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE7_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE8_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE9_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PE10_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PE11_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PE12_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PE13_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PE14_NOPIN) | \
                                     PIN_PUPDR_PULLUP (GPIO_PE15_NOPIN))

#define VAL_GPIOE_ODR               (PIN_ODR_HIGH (GPIO_PE0_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE1_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE2_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE3_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE4_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE5_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE6_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE7_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE8_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE9_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PE10_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PE11_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PE12_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PE13_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PE14_NOPIN) | \
                                     PIN_ODR_HIGH (GPIO_PE15_NOPIN))

#define VAL_GPIOE_AFRL              (PIN_AFIO_AF(GPIO_PE0_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE1_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE2_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE3_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE4_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE5_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE6_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE7_NOPIN, 0U))

#define VAL_GPIOE_AFRH              (PIN_AFIO_AF(GPIO_PE8_NOPIN,  0U) | \
                                     PIN_AFIO_AF(GPIO_PE9_NOPIN,  0U) | \
                                     PIN_AFIO_AF(GPIO_PE10_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE11_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE12_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE13_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE14_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PE15_NOPIN, 0U))

/*
 * 
 * GPIOF setup
 *
 */

#define VAL_GPIOF_MODER             (PIN_MODE_INPUT (GPIO_PF0_OSC_IN ) | \
                                     PIN_MODE_INPUT (GPIO_PF1_OSC_OUT) | \
                                     PIN_MODE_INPUT (GPIO_PF2_NOPIN)   | \
                                     PIN_MODE_INPUT (GPIO_PF3_NOPIN)   | \
                                     PIN_MODE_INPUT (GPIO_PF4_NOPIN)   | \
                                     PIN_MODE_INPUT (GPIO_PF5_NOPIN)   | \
                                     PIN_MODE_INPUT (GPIO_PF6_NOPIN)   | \
                                     PIN_MODE_INPUT (GPIO_PF7_NOPIN)   | \
                                     PIN_MODE_INPUT (GPIO_PF8_NOPIN)   | \
                                     PIN_MODE_INPUT (GPIO_PF9_NOPIN)   | \
                                     PIN_MODE_INPUT (GPIO_PF10_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PF11_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PF12_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PF13_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PF14_NOPIN)  | \
                                     PIN_MODE_INPUT (GPIO_PF15_NOPIN))

#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL (GPIO_PF0_OSC_IN ) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF1_OSC_OUT) | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF2_NOPIN)   | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF3_NOPIN)   | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF4_NOPIN)   | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF5_NOPIN)   | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF6_NOPIN)   | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF7_NOPIN)   | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF8_NOPIN)   | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF9_NOPIN)   | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF10_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF11_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF12_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF13_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF14_NOPIN)  | \
                                     PIN_OTYPE_PUSHPULL (GPIO_PF15_NOPIN))

#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_LOW  (GPIO_PF0_OSC_IN ) | \
                                     PIN_OSPEED_LOW  (GPIO_PF1_OSC_OUT) | \
                                     PIN_OSPEED_LOW  (GPIO_PF2_NOPIN)   | \
                                     PIN_OSPEED_LOW  (GPIO_PF3_NOPIN)   | \
                                     PIN_OSPEED_LOW  (GPIO_PF4_NOPIN)   | \
                                     PIN_OSPEED_LOW  (GPIO_PF5_NOPIN)   | \
                                     PIN_OSPEED_LOW  (GPIO_PF6_NOPIN)   | \
                                     PIN_OSPEED_LOW  (GPIO_PF7_NOPIN)   | \
                                     PIN_OSPEED_LOW  (GPIO_PF8_NOPIN)   | \
                                     PIN_OSPEED_LOW  (GPIO_PF9_NOPIN)   | \
                                     PIN_OSPEED_LOW  (GPIO_PF10_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PF11_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PF12_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PF13_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PF14_NOPIN)  | \
                                     PIN_OSPEED_LOW  (GPIO_PF15_NOPIN))

#define VAL_GPIOF_PUPDR             (PIN_PUPDR_PULLUP (GPIO_PF0_OSC_IN ) | \
                                     PIN_PUPDR_PULLUP (GPIO_PF1_OSC_OUT) | \
                                     PIN_PUPDR_PULLUP (GPIO_PF2_NOPIN)   | \
                                     PIN_PUPDR_PULLUP (GPIO_PF3_NOPIN)   | \
                                     PIN_PUPDR_PULLUP (GPIO_PF4_NOPIN)   | \
                                     PIN_PUPDR_PULLUP (GPIO_PF5_NOPIN)   | \
                                     PIN_PUPDR_PULLUP (GPIO_PF6_NOPIN)   | \
                                     PIN_PUPDR_PULLUP (GPIO_PF7_NOPIN)   | \
                                     PIN_PUPDR_PULLUP (GPIO_PF8_NOPIN)   | \
                                     PIN_PUPDR_PULLUP (GPIO_PF9_NOPIN)   | \
                                     PIN_PUPDR_PULLUP (GPIO_PF10_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PF11_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PF12_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PF13_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PF14_NOPIN)  | \
                                     PIN_PUPDR_PULLUP (GPIO_PF15_NOPIN))

#define VAL_GPIOF_ODR               (PIN_ODR_HIGH (GPIO_PF0_OSC_IN ) | \
                                     PIN_ODR_HIGH (GPIO_PF1_OSC_OUT) | \
                                     PIN_ODR_HIGH (GPIO_PF2_NOPIN)   | \
                                     PIN_ODR_HIGH (GPIO_PF3_NOPIN)   | \
                                     PIN_ODR_HIGH (GPIO_PF4_NOPIN)   | \
                                     PIN_ODR_HIGH (GPIO_PF5_NOPIN)   | \
                                     PIN_ODR_HIGH (GPIO_PF6_NOPIN)   | \
                                     PIN_ODR_HIGH (GPIO_PF7_NOPIN)   | \
                                     PIN_ODR_HIGH (GPIO_PF8_NOPIN)   | \
                                     PIN_ODR_HIGH (GPIO_PF9_NOPIN)   | \
                                     PIN_ODR_HIGH (GPIO_PF10_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PF11_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PF12_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PF13_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PF14_NOPIN)  | \
                                     PIN_ODR_HIGH (GPIO_PF15_NOPIN))

#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIO_PF0_OSC_IN,  0U) | \
                                     PIN_AFIO_AF(GPIO_PF1_OSC_OUT, 0U) | \
                                     PIN_AFIO_AF(GPIO_PF2_NOPIN,   0U) | \
                                     PIN_AFIO_AF(GPIO_PF3_NOPIN,   0U) | \
                                     PIN_AFIO_AF(GPIO_PF4_NOPIN,   0U) | \
                                     PIN_AFIO_AF(GPIO_PF5_NOPIN,   0U) | \
                                     PIN_AFIO_AF(GPIO_PF6_NOPIN,   0U) | \
                                     PIN_AFIO_AF(GPIO_PF7_NOPIN,   0U))

#define VAL_GPIOF_AFRH              (PIN_AFIO_AF(GPIO_PF8_NOPIN,  0U) | \
                                     PIN_AFIO_AF(GPIO_PF9_NOPIN,  0U) | \
                                     PIN_AFIO_AF(GPIO_PF10_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PF11_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PF12_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PF13_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PF14_NOPIN, 0U) | \
                                     PIN_AFIO_AF(GPIO_PF15_NOPIN, 0U))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* BOARD_H */
