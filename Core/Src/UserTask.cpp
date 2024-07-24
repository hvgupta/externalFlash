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
#include "flash.hpp"
#include "gpio.h"
#include "main.h"
#include "task.h"

using namespace Core::Drivers;

StackType_t uxBlinkTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xBlinkTaskTCB;
W25N01::Manager flash;
static uint8_t buffer[20];
uint8_t t_byte = 0;

int numToStr(int num, uint8_t buffer[], int size)
{
    int i = 0;
    while (num != 0)
    {
        buffer[i] = num % 10 + '0';
        num /= 10;
        i++;
    }
    for (int i = 0; i < size / 2; i++)
    {
        char temp            = buffer[i];
        buffer[i]            = buffer[size - i - 1];
        buffer[size - i - 1] = temp;
    }
    buffer[i] = '\0';
    return i;
}

void blink(void *pvPara)
{
    HAL_GPIO_WritePin(LED_ACT_GPIO_Port, LED_ACT_Pin, GPIO_PIN_RESET);
    while (W25N01::isBusy())
        ;
    // flash.EraseChip();

    while (true)
    {
        HAL_GPIO_TogglePin(LED_ACT_GPIO_Port, LED_ACT_Pin);
        HAL_GPIO_TogglePin(LASER_GPIO_Port, LASER_Pin);
        const uint8_t *hmm = reinterpret_cast<const unsigned char *>("The current time is: ");
        uint8_t buffer[20];
        int size = numToStr(xTaskGetTickCount(), buffer, 20) + 1;
        // flash.WriteMemory(0, (uint8_t *)hmm, 22);
        // flash.WriteMemory(0, buffer, size);
        flash.ReadMemory(0, 0, t_byte, buffer, 20);
        vTaskDelay(500);
    }
}

/**
 * @brief Create user tasks
 */
void startUserTasks() { xTaskCreateStatic(blink, "blink", configMINIMAL_STACK_SIZE, NULL, 0, uxBlinkTaskStack, &xBlinkTaskTCB); }
