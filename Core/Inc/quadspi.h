/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    quadspi.h
 * @brief   This file contains all the function prototypes for
 *          the quadspi.c file
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
#ifndef __QUADSPI_H__
#define __QUADSPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

    /* USER CODE BEGIN Includes */

    /* USER CODE END Includes */

    extern QSPI_HandleTypeDef hqspi1;

    /* USER CODE BEGIN Private defines */

    /* USER CODE END Private defines */

    void MX_QUADSPI1_Init(void);

    /* USER CODE BEGIN Prototypes */
    HAL_StatusTypeDef BufferCommand(uint16_t pageAddr, uint16_t command);

    HAL_StatusTypeDef PureCommand(uint16_t command);

    HAL_StatusTypeDef Command_Rx_1DataLine(uint16_t command, uint16_t *buffer, uint16_t addr, uint16_t size);
    HAL_StatusTypeDef Command_Rx_2DataLine(uint16_t command, uint16_t *buffer, uint16_t addr, uint16_t size);

    HAL_StatusTypeDef Command_Tx_4DataLine(uint16_t command, uint16_t *buffer, uint16_t addr, uint16_t size);

    HAL_StatusTypeDef StatusReg_Tx(uint16_t *regAddr, uint16_t *regInfo);
    HAL_StatusTypeDef StatusReg_Rx(uint16_t *regAddr, uint16_t *buffer);
    /* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __QUADSPI_H__ */
