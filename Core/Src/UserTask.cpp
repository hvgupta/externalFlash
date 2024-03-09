/**
 * @file UserTask.cpp
 * @author JIANG Yicheng  RM2023 (EthenJ@outlook.sg)
 * @brief Create user tasks with cpp support
 * @version 0.1
 * @date 2022-08-20
 *
 * @copyright Copyright (c) 2022
 */

#include "FreeRTOS.h"
#include "gpio.h"
#include "main.h"
#include "task.h"

StackType_t uxBlinkTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xBlinkTaskTCB;

void blink(void *pvPara)
{
    HAL_GPIO_WritePin(LED_ACT_GPIO_Port, LED_ACT_Pin, GPIO_PIN_RESET);

    while (true)
    {
        HAL_GPIO_TogglePin(LED_ACT_GPIO_Port, LED_ACT_Pin);
        HAL_GPIO_TogglePin(LASER_GPIO_Port, LASER_Pin);
        vTaskDelay(500);
    }
}

/**
 * @brief Create user tasks
 */
void startUserTasks() { xTaskCreateStatic(blink, "blink", configMINIMAL_STACK_SIZE, NULL, 0, uxBlinkTaskStack, &xBlinkTaskTCB); }
