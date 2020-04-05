/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LA_POS_Pin GPIO_PIN_1
#define LA_POS_GPIO_Port GPIOA
#define ISO_CHUTE_Pin GPIO_PIN_3
#define ISO_CHUTE_GPIO_Port GPIOA
#define ISO_DROGUE_Pin GPIO_PIN_4
#define ISO_DROGUE_GPIO_Port GPIOA
#define DCM_DIR_Pin GPIO_PIN_5
#define DCM_DIR_GPIO_Port GPIOA
#define DCM_PWM_Pin GPIO_PIN_6
#define DCM_PWM_GPIO_Port GPIOA
#define DCM_ON_Pin GPIO_PIN_7
#define DCM_ON_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_0
#define LED_GPIO_Port GPIOB
#define BATT_READ_Pin GPIO_PIN_1
#define BATT_READ_GPIO_Port GPIOB
#define LA_IN1_Pin GPIO_PIN_8
#define LA_IN1_GPIO_Port GPIOA
#define LA_IN2_Pin GPIO_PIN_9
#define LA_IN2_GPIO_Port GPIOA
#define SENSOR_ON_Pin GPIO_PIN_3
#define SENSOR_ON_GPIO_Port GPIOB
#define SENSOR2_Pin GPIO_PIN_4
#define SENSOR2_GPIO_Port GPIOB
#define SENSOR1_Pin GPIO_PIN_5
#define SENSOR1_GPIO_Port GPIOB
#define ACOK_Pin GPIO_PIN_7
#define ACOK_GPIO_Port GPIOB
#define SPKR_Pin GPIO_PIN_8
#define SPKR_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define DCM_SPEED_Port GPIOA
#define DCM_SPEED_Pin GPIO_PIN_0
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
