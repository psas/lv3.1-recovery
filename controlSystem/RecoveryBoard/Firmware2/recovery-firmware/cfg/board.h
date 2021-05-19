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

#define GPIOC_PIN0                  0U
#define GPIOC_PIN1                  1U
#define GPIOC_PIN2                  2U
#define GPIOC_PIN3                  3U
#define GPIOC_PIN4                  4U
#define GPIOC_PIN5                  5U
#define GPIOC_PIN6                  6U
#define GPIOC_PIN7                  7U
#define GPIOC_PIN8                  8U
#define GPIOC_PIN9                  9U
#define GPIOC_PIN10                 10U
#define GPIOC_PIN11                 11U
#define GPIOC_PIN12                 12U
#define GPIOC_PIN13                 13U
#define GPIOC_PIN14                 14U
#define GPIOC_PIN15                 15U

#define GPIOD_PIN0                  0U
#define GPIOD_PIN1                  1U
#define GPIOD_PIN2                  2U
#define GPIOD_PIN3                  3U
#define GPIOD_PIN4                  4U
#define GPIOD_PIN5                  5U
#define GPIOD_PIN6                  6U
#define GPIOD_PIN7                  7U
#define GPIOD_PIN8                  8U
#define GPIOD_PIN9                  9U
#define GPIOD_PIN10                 10U
#define GPIOD_PIN11                 11U
#define GPIOD_PIN12                 12U
#define GPIOD_PIN13                 13U
#define GPIOD_PIN14                 14U
#define GPIOD_PIN15                 15U

#define GPIOE_PIN0                  0U
#define GPIOE_PIN1                  1U
#define GPIOE_PIN2                  2U
#define GPIOE_PIN3                  3U
#define GPIOE_PIN4                  4U
#define GPIOE_PIN5                  5U
#define GPIOE_PIN6                  6U
#define GPIOE_PIN7                  7U
#define GPIOE_PIN8                  8U
#define GPIOE_PIN9                  9U
#define GPIOE_PIN10                 10U
#define GPIOE_PIN11                 11U
#define GPIOE_PIN12                 12U
#define GPIOE_PIN13                 13U
#define GPIOE_PIN14                 14U
#define GPIOE_PIN15                 15U

#define GPIO_F0_OSC_IN              0U
#define GPIO_F1_OSC_OUT             1U
#define GPIOF_PIN2                  2U
#define GPIOF_PIN3                  3U
#define GPIOF_PIN4                  4U
#define GPIOF_PIN5                  5U
#define GPIOF_PIN6                  6U
#define GPIOF_PIN7                  7U
#define GPIOF_PIN8                  8U
#define GPIOF_PIN9                  9U
#define GPIOF_PIN10                 10U
#define GPIOF_PIN11                 11U
#define GPIOF_PIN12                 12U
#define GPIOF_PIN13                 13U
#define GPIOF_PIN14                 14U
#define GPIOF_PIN15                 15U

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
#define LINE_PA10_NC                  PAL_LINE(GPIOA, 10U)
#define LINE_PA11_NC                  PAL_LINE(GPIOA, 11U)
#define LINE_PA12_NC                  PAL_LINE(GPIOA, 12U)
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
 * GPIOA setup:
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
                           PIN_ODR_HIGH  (GPIO_PA14_SWCLK)   | \
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
                           PIN_AFIO_AF (GPIO_PA13_SWDIO,    0U) | \
                           PIN_AFIO_AF (GPIO_PA14_SWCLK,    0U) | \
                           PIN_AFIO_AF (GPIO_PA15_UART2_RX, 1U))

/*
 * 
 * GPIOB setup
 *
 */

#define VAL_GPIOB_MODER   (PIN_MODE_OUTPUT (GPIO_PB0_LED)          | \
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

#define VAL_GPIOB_OTYPER  (PIN_OTYPE_PUSHPULL (GPIO_PB0_LED)          | \
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

#define VAL_GPIOB_OSPEEDR (PIN_OSPEED_LOW  (GPIO_PB0_LED)          | \
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

#define VAL_GPIOB_PUPDR   (PIN_PUPDR_FLOATING (GPIO_PB0_LED)          | \
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

#define VAL_GPIOB_ODR     (PIN_ODR_LOW  (GPIO_PB0_LED)          | \
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

#define VAL_GPIOB_AFRL    (PIN_AFIO_AF (GPIO_PB0_LED,          0U) | \
                           PIN_AFIO_AF (GPIO_PB1_BATT_READ,    0U) | \
                           PIN_AFIO_AF (GPIO_PB2_NOPIN,        0U) | \
                           PIN_AFIO_AF (GPIO_PB3_ROTSENSE_PWR, 0U) | \
                           PIN_AFIO_AF (GPIO_PB4_ROTSENSE2,    0U) | \
                           PIN_AFIO_AF (GPIO_PB5_ROTSENSE1,    0U) | \
                           PIN_AFIO_AF (GPIO_PB6_NC,           0U) | \
                           PIN_AFIO_AF (GPIO_PB7_UMB_ON,       0U))

#define VAL_GPIOB_AFRH    (PIN_AFIO_AF (GPIO_PB8_SPKR,   0U) | \
                           PIN_AFIO_AF (GPIO_PB9_NOPIN,  0U) | \
                           PIN_AFIO_AF (GPIO_PB10_NOPIN, 0U) | \
                           PIN_AFIO_AF (GPIO_PB11_NOPIN, 0U) | \
                           PIN_AFIO_AF (GPIO_PB12_NOPIN, 0U) | \
                           PIN_AFIO_AF (GPIO_PB13_NOPIN, 0U) | \
                           PIN_AFIO_AF (GPIO_PB14_NOPIN, 0U) | \
                           PIN_AFIO_AF (GPIO_PB15_NOPIN, 0U))


/*
 * GPIOC setup:
 *
 * PC0  - PIN0                      (input pullup).
 * PC1  - PIN1                      (input pullup).
 * PC2  - PIN2                      (input pullup).
 * PC3  - PIN3                      (input pullup).
 * PC4  - PIN4                      (input pullup).
 * PC5  - PIN5                      (input pullup).
 * PC6  - PIN6                      (input pullup).
 * PC7  - PIN7                      (input pullup).
 * PC8  - PIN8                      (input pullup).
 * PC9  - PIN9                      (input pullup).
 * PC10 - PIN10                     (input pullup).
 * PC11 - PIN11                     (input pullup).
 * PC12 - PIN12                     (input pullup).
 * PC13 - PIN13                     (input pullup).
 * PC14 - PIN14                     (input pullup).
 * PC15 - PIN15                     (input pullup).
 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN15))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN15))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_HIGH(GPIOC_PIN0) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN1) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN7) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN13) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN14) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN15))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP(GPIOC_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN15))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN15))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN7, 0U))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN15, 0U))

/*
 * GPIOD setup:
 *
 * PD0  - PIN0                      (input pullup).
 * PD1  - PIN1                      (input pullup).
 * PD2  - PIN2                      (input pullup).
 * PD3  - PIN3                      (input pullup).
 * PD4  - PIN4                      (input pullup).
 * PD5  - PIN5                      (input pullup).
 * PD6  - PIN6                      (input pullup).
 * PD7  - PIN7                      (input pullup).
 * PD8  - PIN8                      (input pullup).
 * PD9  - PIN9                      (input pullup).
 * PD10 - PIN10                     (input pullup).
 * PD11 - PIN11                     (input pullup).
 * PD12 - PIN12                     (input pullup).
 * PD13 - PIN13                     (input pullup).
 * PD14 - PIN14                     (input pullup).
 * PD15 - PIN15                     (input pullup).
 */
#define VAL_GPIOD_MODER             (PIN_MODE_INPUT(GPIOD_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN15))
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOD_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN15))
#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_HIGH(GPIOD_PIN0) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN1) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN7) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN13) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN14) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN15))
#define VAL_GPIOD_PUPDR             (PIN_PUPDR_PULLUP(GPIOD_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN15))
#define VAL_GPIOD_ODR               (PIN_ODR_HIGH(GPIOD_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN15))
#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIOD_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN7, 0U))
#define VAL_GPIOD_AFRH              (PIN_AFIO_AF(GPIOD_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN15, 0U))

/*
 * GPIOE setup:
 *
 * PE0  - PIN0                      (input pullup).
 * PE1  - PIN1                      (input pullup).
 * PE2  - PIN2                      (input pullup).
 * PE3  - PIN3                      (input pullup).
 * PE4  - PIN4                      (input pullup).
 * PE5  - PIN5                      (input pullup).
 * PE6  - PIN6                      (input pullup).
 * PE7  - PIN7                      (input pullup).
 * PE8  - PIN8                      (input pullup).
 * PE9  - PIN9                      (input pullup).
 * PE10 - PIN10                     (input pullup).
 * PE11 - PIN11                     (input pullup).
 * PE12 - PIN12                     (input pullup).
 * PE13 - PIN13                     (input pullup).
 * PE14 - PIN14                     (input pullup).
 * PE15 - PIN15                     (input pullup).
 */
#define VAL_GPIOE_MODER             (PIN_MODE_INPUT(GPIOE_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN15))
#define VAL_GPIOE_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOE_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN15))
#define VAL_GPIOE_OSPEEDR           (PIN_OSPEED_HIGH(GPIOE_PIN0) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN1) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN7) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOE_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN13) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN14) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN15))
#define VAL_GPIOE_PUPDR             (PIN_PUPDR_PULLUP(GPIOE_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN15))
#define VAL_GPIOE_ODR               (PIN_ODR_HIGH(GPIOE_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN15))
#define VAL_GPIOE_AFRL              (PIN_AFIO_AF(GPIOE_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN7, 0U))
#define VAL_GPIOE_AFRH              (PIN_AFIO_AF(GPIOE_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN15, 0U))

/*
 * GPIOF setup:
 *
 * PF0  - ARD_D7                    (input pullup).
 * PF1  - ARD_D8                    (input pullup).
 * PF2  - PIN2                      (input pullup).
 * PF3  - PIN3                      (input pullup).
 * PF4  - PIN4                      (input pullup).
 * PF5  - PIN5                      (input pullup).
 * PF6  - PIN6                      (input pullup).
 * PF7  - PIN7                      (input pullup).
 * PF8  - PIN8                      (input pullup).
 * PF9  - PIN9                      (input pullup).
 * PF10 - PIN10                     (input pullup).
 * PF11 - PIN11                     (input pullup).
 * PF12 - PIN12                     (input pullup).
 * PF13 - PIN13                     (input pullup).
 * PF14 - PIN14                     (input pullup).
 * PF15 - PIN15                     (input pullup).
 */
#define VAL_GPIOF_MODER             (PIN_MODE_INPUT(GPIO_F0_OSC_IN) |         \
                                     PIN_MODE_INPUT(GPIO_F1_OSC_OUT) |         \
                                     PIN_MODE_INPUT(GPIOF_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN15))
#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL(GPIO_F0_OSC_IN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIO_F1_OSC_OUT) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN15))
#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_HIGH(GPIO_F0_OSC_IN) |        \
                                     PIN_OSPEED_HIGH(GPIO_F1_OSC_OUT) |        \
                                     PIN_OSPEED_HIGH(GPIOF_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN7) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN13) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN14) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN15))
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_PULLUP(GPIO_F0_OSC_IN) |       \
                                     PIN_PUPDR_PULLUP(GPIO_F1_OSC_OUT) |       \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN15))
#define VAL_GPIOF_ODR               (PIN_ODR_HIGH(GPIO_F0_OSC_IN) |           \
                                     PIN_ODR_HIGH(GPIO_F1_OSC_OUT) |           \
                                     PIN_ODR_HIGH(GPIOF_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN15))
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIO_F0_OSC_IN, 0U) |        \
                                     PIN_AFIO_AF(GPIO_F1_OSC_OUT, 0U) |        \
                                     PIN_AFIO_AF(GPIOF_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN7, 0U))
#define VAL_GPIOF_AFRH              (PIN_AFIO_AF(GPIOF_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN15, 0U))
                                       
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
                                                 
                                                 
                                                 
                                                 
                                                 
                                                 

                                                 
                                                 
                                                 
                                                 
                                                 
                                                 
                                                 
                                                 

                                                 
                                                 
                                                 
                                                 
                                                 
                                                 
                                                 
                                                 
