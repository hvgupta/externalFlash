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
#include "stm32g4xx_hal.h"

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
#define LED_ACT_Pin GPIO_PIN_13
#define LED_ACT_GPIO_Port GPIOC
#define LASER_Pin GPIO_PIN_14
#define LASER_GPIO_Port GPIOC
#define BUTTON_Pin GPIO_PIN_15
#define BUTTON_GPIO_Port GPIOC
#define NRST_Pin GPIO_PIN_10
#define NRST_GPIO_Port GPIOG
#define UART2A_CTS_Pin GPIO_PIN_0
#define UART2A_CTS_GPIO_Port GPIOA
#define UART2A_RTS_Pin GPIO_PIN_1
#define UART2A_RTS_GPIO_Port GPIOA
#define UART2A_TX_Pin GPIO_PIN_2
#define UART2A_TX_GPIO_Port GPIOA
#define UART2A_RX_Pin GPIO_PIN_3
#define UART2A_RX_GPIO_Port GPIOA
#define CS_IMU_Pin GPIO_PIN_4
#define CS_IMU_GPIO_Port GPIOA
#define UART1A_TX_Pin GPIO_PIN_4
#define UART1A_TX_GPIO_Port GPIOC
#define UART1A_RX_Pin GPIO_PIN_5
#define UART1A_RX_GPIO_Port GPIOC
#define DRDY_MAG_Pin GPIO_PIN_0
#define DRDY_MAG_GPIO_Port GPIOB
#define DRDY_IMU_Pin GPIO_PIN_1
#define DRDY_IMU_GPIO_Port GPIOB
#define CS_MAG_Pin GPIO_PIN_2
#define CS_MAG_GPIO_Port GPIOB
#define IMU_HEAT_Pin GPIO_PIN_7
#define IMU_HEAT_GPIO_Port GPIOE
#define UART3B_RX_Pin GPIO_PIN_15
#define UART3B_RX_GPIO_Port GPIOE
#define UART3B_TX_Pin GPIO_PIN_10
#define UART3B_TX_GPIO_Port GPIOB
#define UART3A_TX_Pin GPIO_PIN_8
#define UART3A_TX_GPIO_Port GPIOD
#define UART3A_RX_Pin GPIO_PIN_9
#define UART3A_RX_GPIO_Port GPIOD
#define UART4_DE_Pin GPIO_PIN_15
#define UART4_DE_GPIO_Port GPIOA
#define UART4_TX_Pin GPIO_PIN_10
#define UART4_TX_GPIO_Port GPIOC
#define UART4_RX_Pin GPIO_PIN_11
#define UART4_RX_GPIO_Port GPIOC
#define UART5_TX_Pin GPIO_PIN_12
#define UART5_TX_GPIO_Port GPIOC
#define UART5_RX_Pin GPIO_PIN_2
#define UART5_RX_GPIO_Port GPIOD
#define UART2B_TX_Pin GPIO_PIN_5
#define UART2B_TX_GPIO_Port GPIOD
#define UART2B_RX_Pin GPIO_PIN_6
#define UART2B_RX_GPIO_Port GPIOD
#define UART1B_TX_Pin GPIO_PIN_6
#define UART1B_TX_GPIO_Port GPIOB
#define UART1B_RX_Pin GPIO_PIN_7
#define UART1B_RX_GPIO_Port GPIOB
#define BOOT0_Pin GPIO_PIN_8
#define BOOT0_GPIO_Port GPIOB
#define UART1C_TX_Pin GPIO_PIN_0
#define UART1C_TX_GPIO_Port GPIOE
#define UART1C_RX_Pin GPIO_PIN_1
#define UART1C_RX_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
