/**
 * @file UserTask.cpp
 * @author JIANG Yicheng  RM2023 (EthenJ@outlook.sg)
 * @brief Create user tasks with cpp support
 * @version 0.1
 * @date 2022-08-20
 *
 * @copyright Copyright (c) 2022
 */

#include "DJIMotorTypeA.hpp"
#include "DR16.hpp"
#include "FDCANManager.hpp"
#include "FreeRTOS.h"
#include "gpio.h"
#include "main.h"
#include "task.h"

StackType_t uxBlinkTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xBlinkTaskTCB;

void blink(void *pvPara)
{
    while (true)
    {
        HAL_GPIO_TogglePin(LED_ACT_GPIO_Port, LED_ACT_Pin);
        vTaskDelay(500);
    }
}

using namespace Core::Drivers;
DJIMotorTypeA testMotor(0x201, 2, 19, 1);

/**
 * @brief Create user tasks
 */

void startUserTasks()
{
    DR16::init();
    CANManager::managers[1].init(&hfdcan2);
    CANManager::managers[0].init(&hfdcan1);
    CANManager::managers[2].init(&hfdcan3);
    DJIMotor::init();
    xTaskCreateStatic(blink, "blink", configMINIMAL_STACK_SIZE, NULL, 0, uxBlinkTaskStack, &xBlinkTaskTCB);
}
