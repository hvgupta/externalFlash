/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    quadspi.c
 * @brief   This file provides code for the configuration
 *          of the QUADSPI instances.
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
/* Includes ------------------------------------------------------------------*/
#include "quadspi.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

QSPI_HandleTypeDef hqspi1;
DMA_HandleTypeDef hdma_quadspi;

/* QUADSPI1 init function */
void MX_QUADSPI1_Init(void)
{
    /* USER CODE BEGIN QUADSPI1_Init 0 */

    /* USER CODE END QUADSPI1_Init 0 */

    /* USER CODE BEGIN QUADSPI1_Init 1 */

    /* USER CODE END QUADSPI1_Init 1 */
    hqspi1.Instance                = QUADSPI;
    hqspi1.Init.ClockPrescaler     = 1;
    hqspi1.Init.FifoThreshold      = 4;
    hqspi1.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi1.Init.FlashSize          = 26;
    hqspi1.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
    hqspi1.Init.ClockMode          = QSPI_CLOCK_MODE_0;
    hqspi1.Init.FlashID            = QSPI_FLASH_ID_2;
    hqspi1.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;
    if (HAL_QSPI_Init(&hqspi1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN QUADSPI1_Init 2 */

    /* USER CODE END QUADSPI1_Init 2 */
}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef *qspiHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct       = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    if (qspiHandle->Instance == QUADSPI)
    {
        /* USER CODE BEGIN QUADSPI_MspInit 0 */

        /* USER CODE END QUADSPI_MspInit 0 */

        /** Initializes the peripherals clocks
         */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_QSPI;
        PeriphClkInit.QspiClockSelection   = RCC_QSPICLKSOURCE_SYSCLK;

        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }

        /* QUADSPI clock enable */
        __HAL_RCC_QSPI_CLK_ENABLE();

        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**QUADSPI1 GPIO Configuration
        PF10     ------> QUADSPI1_CLK
        PC2     ------> QUADSPI1_BK2_IO1
        PC3     ------> QUADSPI1_BK2_IO2
        PD3     ------> QUADSPI1_BK2_NCS
        PD4     ------> QUADSPI1_BK2_IO0
        PD7     ------> QUADSPI1_BK2_IO3
        */
        GPIO_InitStruct.Pin       = GPIO_PIN_10;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_7;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* QUADSPI DMA Init */
        /* QUADSPI Init */
        hdma_quadspi.Instance                 = DMA2_Channel3;
        hdma_quadspi.Init.Request             = DMA_REQUEST_QUADSPI;
        hdma_quadspi.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        hdma_quadspi.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_quadspi.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_quadspi.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_quadspi.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        hdma_quadspi.Init.Mode                = DMA_NORMAL;
        hdma_quadspi.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
        if (HAL_DMA_Init(&hdma_quadspi) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(qspiHandle, hdma, hdma_quadspi);

        /* QUADSPI interrupt Init */
        HAL_NVIC_SetPriority(QUADSPI_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
        /* USER CODE BEGIN QUADSPI_MspInit 1 */

        /* USER CODE END QUADSPI_MspInit 1 */
    }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *qspiHandle)
{
    if (qspiHandle->Instance == QUADSPI)
    {
        /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

        /* USER CODE END QUADSPI_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_QSPI_CLK_DISABLE();

        /**QUADSPI1 GPIO Configuration
        PF10     ------> QUADSPI1_CLK
        PC2     ------> QUADSPI1_BK2_IO1
        PC3     ------> QUADSPI1_BK2_IO2
        PD3     ------> QUADSPI1_BK2_NCS
        PD4     ------> QUADSPI1_BK2_IO0
        PD7     ------> QUADSPI1_BK2_IO3
        */
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_10);

        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2 | GPIO_PIN_3);

        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_7);

        /* QUADSPI DMA DeInit */
        HAL_DMA_DeInit(qspiHandle->hdma);

        /* QUADSPI interrupt Deinit */
        HAL_NVIC_DisableIRQ(QUADSPI_IRQn);
        /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

        /* USER CODE END QUADSPI_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
HAL_StatusTypeDef BufferCommand(uint16_t pageAddr, uint16_t command)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction         = command;

    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address     = pageAddr;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;

    sCommand.DdrMode          = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi1, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef PureCommand(uint16_t command)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction         = command;

    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AddressSize = QSPI_ADDRESS_8_BITS;
    sCommand.Address     = 0;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;

    sCommand.DdrMode          = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi1, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef Command_Rx_1DataLine_addr(uint16_t command, uint8_t *buffer, uint16_t addr, uint16_t size)
{
    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction     = command;

    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_16_BITS;
    sCommand.Address     = addr;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

    sCommand.DataMode    = QSPI_DATA_1_LINE;
    sCommand.NbData      = size;
    sCommand.DummyCycles = 8;

    sCommand.DdrMode          = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi1, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive_DMA(&hqspi1, buffer) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef Command_Rx_1DataLine(uint16_t command, uint8_t *buffer, uint16_t size, uint16_t dummyCycle)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction         = command;

    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AddressSize = QSPI_ADDRESS_8_BITS;
    sCommand.Address     = 0x0U;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

    sCommand.DataMode    = QSPI_DATA_1_LINE;
    sCommand.NbData      = size;
    sCommand.DummyCycles = dummyCycle;

    sCommand.DdrMode          = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi1, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive_DMA(&hqspi1, buffer) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef Command_Rx_2DataLine(uint16_t command, uint8_t *buffer, uint16_t addr, uint16_t size)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction         = command;

    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_16_BITS;
    sCommand.Address     = addr;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

    sCommand.DataMode    = QSPI_DATA_2_LINES;
    sCommand.NbData      = size;
    sCommand.DummyCycles = 8;

    sCommand.DdrMode          = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi1, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive_DMA(&hqspi1, buffer) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef Command_Tx_4DataLine(uint16_t command, uint8_t *buffer, uint16_t addr, uint16_t size)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction         = command;

    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_16_BITS;
    sCommand.Address     = addr;

    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.AlternateBytes     = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

    sCommand.DummyCycles = 0;
    sCommand.DataMode    = QSPI_DATA_4_LINES;
    sCommand.NbData      = size;

    sCommand.DdrMode          = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi1, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if (HAL_QSPI_Transmit_DMA(&hqspi1, buffer) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef StatusReg_Tx(uint16_t command, uint16_t regAddr, uint8_t *data)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction         = command;

    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_8_BITS;
    sCommand.Address     = regAddr;

    sCommand.DummyCycles = 0;
    sCommand.DataMode    = QSPI_DATA_1_LINE;
    sCommand.NbData      = 1;

    sCommand.DdrMode          = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi1, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if (HAL_QSPI_Transmit_DMA(&hqspi1, data) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef StatusReg_Rx(uint16_t command, uint16_t regAddr, uint8_t *buffer)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction         = command;

    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_8_BITS;
    sCommand.Address     = regAddr;

    sCommand.DummyCycles = 0;
    sCommand.DataMode    = QSPI_DATA_1_LINE;
    sCommand.NbData      = 1;

    sCommand.DdrMode          = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;
    if (HAL_QSPI_Command(&hqspi1, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if (HAL_QSPI_Receive_DMA(&hqspi1, buffer) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/* USER CODE END 1 */
