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
 * Setup for STMicroelectronics STM32 Nucleo64-F091RC board.
 */

/*
 * Board identifier.
 */
#define BOARD_ST_NUCLEO64_F091RC
#define BOARD_NAME                  "STMicroelectronics STM32 Nucleo64-F091RC"

/*
 * Board oscillators-related settings.
 * NOTE: LSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0U
#endif

#define STM32_LSEDRV                (3U << 3U)

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                16000000U
#endif



/*
 * MCU type as defined in the ST header.
 */
#define STM32F091xC

/*
 * IO pins assignments.
 */
#define GPIOA_BATTREAD                0U
#define GPIOA_ADC1_IN0              0U
#define GPIOA_UNUSEDPA1                1U
#define GPIOA_ADC1_IN1              1U
#define GPIOA_USART2_TX             2U
#define GPIOA_USART2_RX             3U
#define GPIOA_ISO_MAIN                4U
#define GPIOA_ADC1_IN4              4U
#define GPIOA_ISO_DROGUE             5U
#define GPIOA_ARD_D13               5U
#define GPIOA_DEPLOY1               6U
#define GPIOA_DEPLOY2               7U
#define GPIOA_UNUSEDPA8                8U
#define GPIOA_CAN_SILENT                9U
#define GPIOA_CAN_SHDN                10U
#define GPIOA_PIN11                 11U
#define GPIOA_PIN12                 12U
#define GPIOA_SWDIO                 13U
#define GPIOA_SWCLK                 14U
#define GPIOA_UNUSEDPA15                 15U

#define GPIOB_HALL1                0U
#define GPIOB_ADC1_IN8              0U
#define GPIOB_HALL2                  1U
#define GPIOB_UNUSEDPB2             2U
#define GPIOB_SWO                   3U
#define GPIOB_UNUSEDPB3             3U
#define GPIOB_UNUSEDPB4             4U
#define GPIOB_ARD_D4                5U
#define GPIOB_UNUSEDPB6             6U
#define GPIOB_UNUSEDPB7             7U
#define GPIOB_UNUSEDPB8             8U
#define GPIOB_UNUSEDPB9             9U
#define GPIOB_UNUSEDPB10            10U
#define GPIOB_UNUSEDPB11            11U
#define GPIOB_UMB                   12U
#define GPIOB_UNUSEDPB13            13U
#define GPIOB_LED                   14U
#define GPIOB_SPEAKER                 15U

#define GPIOC_ARD_A5                0U
#define GPIOC_ADC1_IN11             0U
#define GPIOC_ARD_A4                1U
#define GPIOC_ADC1_IN10             1U
#define GPIOC_PIN2                  2U
#define GPIOC_PIN3                  3U
#define GPIOC_PIN4                  4U
#define GPIOC_PIN5                  5U
#define GPIOC_PIN6                  6U
#define GPIOC_ARD_D9                7U
#define GPIOC_PIN8                  8U
#define GPIOC_PIN9                  9U
#define GPIOC_PIN10                 10U
#define GPIOC_PIN11                 11U
#define GPIOC_PIN12                 12U
#define GPIOC_BUTTON                13U
#define GPIOC_OSC32_IN              14U
#define GPIOC_OSC32_OUT             15U

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

#define GPIOF_OSC_IN                0U
#define GPIOF_OSC_OUT               1U
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

 /*A PORTS*/
#define LINE_BATTREAD               PAL_LINE(GPIOA, 0U)
#define LINE_USART2_TX              PAL_LINE(GPIOA, 2U)
#define LINE_USART2_RX              PAL_LINE(GPIOA, 3U)
#define LINE_ISO_MAIN               PAL_LINE(GPIOA, 4U)
#define LINE_ISO_DROGUE             PAL_LINE(GPIOA, 5U)
#define LINE_DEPLOY1                PAL_LINE(GPIOA, 6U)
#define LINE_DEPLOY2                PAL_LINE(GPIOA, 7U)
#define LINE_ARD_D8                 PAL_LINE(GPIOA, 8U)
#define LINE_CAN_SILENT             PAL_LINE(GPIOA, 9U)
#define LINE_CAN_SHDN               PAL_LINE(GPIOA, 10U)

/*B PORTS*/
#define LINE_HALL1                  PAL_LINE(GPIOB, 0U)
#define LINE_HALL2                  PAL_LINE(GPIOB, 1U)
#define LINE_LED                    PAL_LINE(GPIOB, 14U)
#define LINE_LA_DRV12               PAL_LINE(GPIOB, 12U)
#define LINE_SPEAKER                PAL_LINE(GPIOB, 15U)

/*UNUSED PORTS GPIO*/
//A
#define LINE_UNUSEDPA1             PAL_LINE(GPIOA, 1U)
#define LINE_UNUSEDPA11            PAL_LINE(GPIOA, 11U)
#define LINE_UNUSEDPA8             PAL_LINE(GPIOA, 8U)
//B
#define LINE_UNUSEDPB2              PAL_LINE(GPIOB, 2U)
#define LINE_UNUSEDPB3              PAL_LINE(GPIOB, 3U)
#define LINE_UNUSEDPB4              PAL_LINE(GPIOB, 4U)
#define LINE_UNUSEDPB5              PAL_LINE(GPIOB, 5U)
#define LINE_UNUSEDPB6              PAL_LINE(GPIOB, 6U)
#define LINE_UNUSEDPB7              PAL_LINE(GPIOB, 7U)
#define LINE_UNUSEDPB8              PAL_LINE(GPIOB, 8U)
#define LINE_UNUSEDPB9              PAL_LINE(GPIOB, 9U)
#define LINE_UNUSEDPB10             PAL_LINE(GPIOB, 10U)
#define LINE_UNUSEDPB11             PAL_LINE(GPIOB, 11U)
#define LINE_UNUSEDPB13             PAL_LINE(GPIOB, 13U)

/*C PORTS*/
#define LINE_BUTTON                 PAL_LINE(GPIOC, 13U)
#define LINE_OSC32_IN               PAL_LINE(GPIOC, 14U)
#define LINE_OSC32_OUT              PAL_LINE(GPIOC, 15U)

/*F PORTS*/
#define LINE_OSC_IN                 PAL_LINE(GPIOF, 0U)
#define LINE_OSC_OUT                PAL_LINE(GPIOF, 1U)

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
 * PA0  - BATTREAD                  (input floating).
 * PA1  - UNUSEDPA1                 (output low).
 * PA2  - ARD_D1, USART2_TX         (alternate 1).
 * PA3  - ARD_D0, USART2_RX         (alternate 1).
 * PA4  - ARD_A2 ADC1_IN4           (input pullup).
 * PA5  - ISO_DROGUE                (output pushpull high).
 * PA6  - ARD_D12, DEPLOY1          (input pullup).
 * PA7  - ARD_D11, DEPLOY2          (input pullup).
 * PA8  - UNUSEDPA8                 (output low).
 * PA9  - ARD_D8, CAN_SILENT        (input pullup).
 * PA10 - ARD_D2, CAN_SHDN          (input pullup).
 * PA11 - PIN11                     (input pullup).
 * PA12 - PIN12                     (input pullup).
 * PA13 - SWDIO                     (alternate 0).
 * PA14 - SWCLK                     (alternate 0).
 * PA15 - PIN15, UNUSEDPA15         (output low).
 */
#define VAL_GPIOA_MODER             (PIN_MODE_ANALOG(GPIOA_BATTREAD) |         \
                                     PIN_MODE_OUTPUT(GPIOA_UNUSEDPA1) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_USART2_TX) |     \
                                     PIN_MODE_ALTERNATE(GPIOA_USART2_RX) |     \
                                     PIN_MODE_INPUT(GPIOA_ISO_MAIN) |         \
                                     PIN_MODE_INPUT(GPIOA_ISO_DROGUE) |     \
                                     PIN_MODE_INPUT(GPIOA_DEPLOY1) |        \
                                     PIN_MODE_INPUT(GPIOA_DEPLOY2) |        \
                                     PIN_MODE_OUTPUT(GPIOA_UNUSEDPA8) |         \
                                     PIN_MODE_INPUT(GPIOA_CAN_SILENT) |         \
                                     PIN_MODE_INPUT(GPIOA_CAN_SHDN) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |      \
                                     PIN_MODE_OUTPUT(GPIOA_UNUSEDPA15)| \
                                     PIN_MODE_ALTERNATE(GPIOA_SWCLK))      
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_BATTREAD) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_USART2_TX) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_USART2_RX) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_ISO_MAIN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_ISO_DROGUE) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_DEPLOY1) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOA_DEPLOY2) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOA_UNUSEDPA8) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_CAN_SILENT) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_CAN_SHDN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDIO) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWCLK))   
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_HIGH(GPIOA_BATTREAD) |        \
                                     PIN_OSPEED_LOW(GPIOA_USART2_TX) |         \
                                     PIN_OSPEED_LOW(GPIOA_USART2_RX) |         \
                                     PIN_OSPEED_HIGH(GPIOA_ISO_MAIN) |        \
                                     PIN_OSPEED_LOW(GPIOA_ISO_DROGUE) |      \
                                     PIN_OSPEED_HIGH(GPIOA_DEPLOY1) |       \
                                     PIN_OSPEED_HIGH(GPIOA_DEPLOY2) |       \
                                     PIN_OSPEED_LOW(GPIOA_UNUSEDPA8) |        \
                                     PIN_OSPEED_HIGH(GPIOA_CAN_SILENT) |        \
                                     PIN_OSPEED_HIGH(GPIOA_CAN_SHDN) |        \
                                     PIN_OSPEED_HIGH(GPIOA_SWDIO) |         \
                                     PIN_OSPEED_HIGH(GPIOA_SWCLK) |         \
                                     PIN_OSPEED_HIGH(GPIOA_UNUSEDPA15))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_FLOATING(GPIOA_BATTREAD) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_UNUSEDPA1) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_USART2_TX) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_USART2_RX) |     \
                                     PIN_PUPDR_PULLUP(GPIOA_ISO_MAIN) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_ISO_MAIN) |  \
                                     PIN_PUPDR_PULLUP(GPIOA_DEPLOY1) |      \
                                     PIN_PUPDR_PULLUP(GPIOA_DEPLOY2) |      \
                                     PIN_PUPDR_PULLUP(GPIOA_UNUSEDPA8) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_CAN_SILENT) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_CAN_SHDN) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_SWDIO) |        \
                                     PIN_PUPDR_PULLDOWN(GPIOA_SWCLK) |      \
                                     PIN_PUPDR_PULLUP(GPIOA_UNUSEDPA15))
#define VAL_GPIOA_ODR               (PIN_ODR_HIGH(GPIOA_BATTREAD) |           \
                                     PIN_ODR_HIGH(GPIOA_UNUSEDPA1) |           \
                                     PIN_ODR_HIGH(GPIOA_USART2_TX) |           \
                                     PIN_ODR_HIGH(GPIOA_USART2_RX) |           \
                                     PIN_ODR_LOW(GPIOA_ISO_MAIN) |         \
                                     PIN_ODR_HIGH(GPIOA_DEPLOY1) |          \
                                     PIN_ODR_HIGH(GPIOA_DEPLOY2) |          \
                                     PIN_ODR_LOW(GPIOA_UNUSEDPA8) |           \
                                     PIN_ODR_HIGH(GPIOA_CAN_SILENT) |           \
                                     PIN_ODR_HIGH(GPIOA_CAN_SHDN) |           \
                                     PIN_ODR_HIGH(GPIOA_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOA_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOA_SWDIO) |            \
                                     PIN_ODR_HIGH(GPIOA_SWCLK) |            \
                                     PIN_ODR_HIGH(GPIOA_UNUSEDPA15)| \
                                     PIN_ODR_HIGH(GPIOA_ISO_MAIN)) 
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_BATTREAD, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_UNUSEDPA1, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_USART2_TX, 1U) |        \
                                     PIN_AFIO_AF(GPIOA_USART2_RX, 1U) |        \
                                     PIN_AFIO_AF(GPIOA_ISO_MAIN, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_ISO_MAIN, 0U) |     \
                                     PIN_AFIO_AF(GPIOA_DEPLOY1, 0U) |       \
                                     PIN_AFIO_AF(GPIOA_DEPLOY2, 0U))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_UNUSEDPA8, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_CAN_SILENT, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_CAN_SHDN, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_SWDIO, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_SWCLK, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_UNUSEDPA15, 0U))

/*
 * GPIOB setup:
 *
 * PB0  - ARD_A3, HALL1             (analog pullup).
 * PB1  - PIN1, HALL2               (analog pullup).
 * PB2  - UNUSEDPB2                 (input pullup).
 * PB3  - UNUSEDPB3                 (alternate 0).
 * PB4  - UNUSEDPB4                 (input pullup).
 * PB5  - UNUSEDPB5                 (input pullup).
 * PB6  - UNUSEDPB6                 (input pullup).
 * PB7  - UNUSEDPB7                 (input pullup).
 * PB8  - UNUSEDPB8                 (input pullup).
 * PB9  - UNUSEDPB9                 (input pullup).
 * PB10 - UNUSEDPB10                (input pullup).
 * PB11 - UNUSEDPB11                (input pullup).
 * PB12 - UMB                       (input pullup).
 * PB13 - UNUSEDPB13                (input pullup).
 * PB14 - LED                       (output pullup).
 * PB15 - SPEAKER                   (output pullup).
 */
#define VAL_GPIOB_MODER             (PIN_MODE_ANALOG(GPIOB_HALL1) |         \
                                     PIN_MODE_ANALOG(GPIOB_HALL2) |           \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB2) |           \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB3) |        \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB4) |         \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB4) |         \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB6) |        \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB7) |           \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB8) |        \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB9) |        \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB10) |         \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB11) |          \
                                     PIN_MODE_OUTPUT(GPIOB_UMB) |          \
                                     PIN_MODE_OUTPUT(GPIOB_UNUSEDPB13) |          \
                                     PIN_MODE_OUTPUT(GPIOB_LED) |          \
                                     PIN_MODE_OUTPUT(GPIOB_SPEAKER))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_HALL1) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_HALL2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB3) |        \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB4) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB4) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB6) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB8) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB9) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB10) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UMB) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UNUSEDPB13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_LED) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPEAKER))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_HIGH(GPIOB_HALL1) |        \
                                     PIN_OSPEED_HIGH(GPIOB_HALL2) |          \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB2) |          \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB3) |           \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB4) |        \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB4) |        \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB6) |       \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB7) |          \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB8) |       \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB9) |       \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB10) |        \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB11) |         \
                                     PIN_OSPEED_HIGH(GPIOB_UMB) |         \
                                     PIN_OSPEED_HIGH(GPIOB_UNUSEDPB13) |         \
                                     PIN_OSPEED_LOW(GPIOB_LED) |         \
                                     PIN_OSPEED_HIGH(GPIOB_SPEAKER))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_FLOATING(GPIOB_HALL1) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_HALL2) |         \
                                     PIN_PUPDR_FLOATING(GPIOB_UNUSEDPB2) |         \
                                     PIN_PUPDR_FLOATING(GPIOB_UNUSEDPB3) |          \
                                     PIN_PUPDR_PULLUP(GPIOB_UNUSEDPB4) |       \
                                     PIN_PUPDR_PULLUP(GPIOB_UNUSEDPB4) |       \
                                     PIN_PUPDR_PULLUP(GPIOB_UNUSEDPB6) |      \
                                     PIN_PUPDR_PULLUP(GPIOB_UNUSEDPB7) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_UNUSEDPB8) |      \
                                     PIN_PUPDR_PULLUP(GPIOB_UNUSEDPB9) |      \
                                     PIN_PUPDR_PULLUP(GPIOB_UNUSEDPB10) |       \
                                     PIN_PUPDR_PULLUP(GPIOB_UNUSEDPB11) |        \
                                     PIN_PUPDR_FLOATING(GPIOB_UMB) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_UNUSEDPB13) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_LED) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_SPEAKER))
#define VAL_GPIOB_ODR               (PIN_ODR_HIGH(GPIOB_HALL1) |           \
                                     PIN_ODR_LOW(GPIOB_HALL2) |             \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB2) |             \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB3) |              \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB4) |           \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB4) |           \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB6) |          \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB7) |             \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB8) |          \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB9) |          \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB10) |           \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB11) |            \
                                     PIN_ODR_LOW(GPIOB_UMB) |            \
                                     PIN_ODR_LOW(GPIOB_UNUSEDPB13) |            \
                                     PIN_ODR_HIGH(GPIOB_LED) |            \
                                     PIN_ODR_HIGH(GPIOB_SPEAKER))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_HALL1, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_HALL2, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB2, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB3, 0U) |           \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB4, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB4, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB6, 0U) |       \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB7, 0U))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_UNUSEDPB8, 0U) |       \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB9, 0U) |       \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB10, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB11, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_UMB, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_UNUSEDPB13, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_LED, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_SPEAKER, 0U))

/*
 * GPIOC setup:
 *
 * PC0  - ARD_A5 ADC1_IN11          (input pullup).
 * PC1  - ARD_A4 ADC1_IN10          (input pullup).
 * PC2  - PIN2                      (input pullup).
 * PC3  - PIN3                      (input pullup).
 * PC4  - PIN4                      (input pullup).
 * PC5  - PIN5                      (input pullup).
 * PC6  - PIN6                      (input pullup).
 * PC7  - ARD_D9                    (input pullup).
 * PC8  - PIN8                      (input pullup).
 * PC9  - PIN9                      (input pullup).
 * PC10 - PIN10                     (input pullup).
 * PC11 - PIN11                     (input pullup).
 * PC12 - PIN12                     (input pullup).
 * PC13 - BUTTON                    (input floating).
 * PC14 - OSC32_IN                  (input floating).
 * PC15 - OSC32_OUT                 (input floating).
 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_ARD_A5) |         \
                                     PIN_MODE_INPUT(GPIOC_ARD_A4) |         \
                                     PIN_MODE_INPUT(GPIOC_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOC_ARD_D9) |         \
                                     PIN_MODE_OUTPUT(GPIOC_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOC_BUTTON) |         \
                                     PIN_MODE_INPUT(GPIOC_OSC32_IN) |       \
                                     PIN_MODE_INPUT(GPIOC_OSC32_OUT))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_ARD_A5) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_ARD_A4) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_ARD_D9) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_BUTTON) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_OSC32_IN) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOC_OSC32_OUT))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_HIGH(GPIOC_ARD_A5) |        \
                                     PIN_OSPEED_HIGH(GPIOC_ARD_A4) |        \
                                     PIN_OSPEED_HIGH(GPIOC_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOC_ARD_D9) |        \
                                     PIN_OSPEED_HIGH(GPIOC_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOC_BUTTON) |        \
                                     PIN_OSPEED_HIGH(GPIOC_OSC32_IN) |      \
                                     PIN_OSPEED_HIGH(GPIOC_OSC32_OUT))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP(GPIOC_ARD_A5) |       \
                                     PIN_PUPDR_PULLUP(GPIOC_ARD_A4) |       \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_ARD_D9) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN12) |        \
                                     PIN_PUPDR_FLOATING(GPIOC_BUTTON) |     \
                                     PIN_PUPDR_FLOATING(GPIOC_OSC32_IN) |   \
                                     PIN_PUPDR_FLOATING(GPIOC_OSC32_OUT))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_ARD_A5) |           \
                                     PIN_ODR_HIGH(GPIOC_ARD_A4) |           \
                                     PIN_ODR_HIGH(GPIOC_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOC_ARD_D9) |           \
                                     PIN_ODR_LOW(GPIOC_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOC_BUTTON) |           \
                                     PIN_ODR_HIGH(GPIOC_OSC32_IN) |         \
                                     PIN_ODR_HIGH(GPIOC_OSC32_OUT))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_ARD_A5, 0U) |        \
                                     PIN_AFIO_AF(GPIOC_ARD_A4, 0U) |        \
                                     PIN_AFIO_AF(GPIOC_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_ARD_D9, 0U))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_BUTTON, 0U) |        \
                                     PIN_AFIO_AF(GPIOC_OSC32_IN, 0U) |      \
                                     PIN_AFIO_AF(GPIOC_OSC32_OUT, 0U))

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
 * PF0  - OSC_IN                    (input floating).
 * PF1  - OSC_OUT                   (input floating).
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
#define VAL_GPIOF_MODER             (PIN_MODE_INPUT(GPIOF_OSC_IN) |         \
                                     PIN_MODE_INPUT(GPIOF_OSC_OUT) |        \
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
#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOF_OSC_IN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_OSC_OUT) |    \
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
#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_HIGH(GPIOF_OSC_IN) |        \
                                     PIN_OSPEED_HIGH(GPIOF_OSC_OUT) |       \
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
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_FLOATING(GPIOF_OSC_IN) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_OSC_OUT) |    \
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
#define VAL_GPIOF_ODR               (PIN_ODR_HIGH(GPIOF_OSC_IN) |           \
                                     PIN_ODR_HIGH(GPIOF_OSC_OUT) |          \
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
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIOF_OSC_IN, 0U) |        \
                                     PIN_AFIO_AF(GPIOF_OSC_OUT, 0U) |       \
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
