/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32h7xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define JOY_RIGHT_Pin GPIO_PIN_5
#define JOY_RIGHT_GPIO_Port GPIOK
#define JOY_RIGHT_EXTI_IRQn EXTI9_5_IRQn
#define JOY_LEFT_Pin GPIO_PIN_4
#define JOY_LEFT_GPIO_Port GPIOK
#define JOY_LEFT_EXTI_IRQn EXTI4_IRQn
#define JOY_UP_Pin GPIO_PIN_6
#define JOY_UP_GPIO_Port GPIOK
#define JOY_UP_EXTI_IRQn EXTI9_5_IRQn
#define JOY_DOWN_Pin GPIO_PIN_3
#define JOY_DOWN_GPIO_Port GPIOK
#define JOY_DOWN_EXTI_IRQn EXTI3_IRQn
#define TOUCH_INTERRUPT_Pin GPIO_PIN_7
#define TOUCH_INTERRUPT_GPIO_Port GPIOK
#define TOUCH_INTERRUPT_EXTI_IRQn EXTI9_5_IRQn
#define LCD_BACKLIGHT_Pin GPIO_PIN_12
#define LCD_BACKLIGHT_GPIO_Port GPIOJ
#define USER_BUTTON_Pin GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOC
#define USER_BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define LED1_Pin GPIO_PIN_12
#define LED1_GPIO_Port GPIOI
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOI
#define LED3_Pin GPIO_PIN_14
#define LED3_GPIO_Port GPIOI
#define DSI_RESET_Pin GPIO_PIN_3
#define DSI_RESET_GPIO_Port GPIOG
#define JOY_SELECT_Pin GPIO_PIN_2
#define JOY_SELECT_GPIO_Port GPIOK
#define JOY_SELECT_EXTI_IRQn EXTI2_IRQn
#define LED4_Pin GPIO_PIN_15
#define LED4_GPIO_Port GPIOI
#define ADC3_IN0_Pin GPIO_PIN_2
#define ADC3_IN0_GPIO_Port GPIOC
#define ADC3_IN1_Pin GPIO_PIN_3
#define ADC3_IN1_GPIO_Port GPIOC
#define LCD_TE_Pin GPIO_PIN_2
#define LCD_TE_GPIO_Port GPIOJ

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
