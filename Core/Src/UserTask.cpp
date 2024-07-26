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

W25N01::Manager flash;
static uint8_t buffer[20];
uint8_t t_byte = 0, t_page = 0, t_block = 0;
uint8_t hmm[64];

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
    for (int i = 0; i < 64; i++)
    {
        hmm[i] = i;
    }
    HAL_GPIO_WritePin(LED_ACT_GPIO_Port, LED_ACT_Pin, GPIO_PIN_RESET);
    while (W25N01::isBusy())
        ;
    flash.EraseChip();

    while (true)
    {
        if (W25N01::isBusy())
        {
            continue;
        }
        HAL_GPIO_TogglePin(LED_ACT_GPIO_Port, LED_ACT_Pin);
        HAL_GPIO_TogglePin(LASER_GPIO_Port, LASER_Pin);
        // const uint8_t *hmm = reinterpret_cast<const unsigned char *>("The current time is: ");
        // uint8_t buffer[20];
        // int size = numToStr(xTaskGetTickCount(), buffer, 20) + 1;
        flash.WriteMemory(0, hmm, 4);
        // flash.WriteMemory(0, buffer, size);
        vTaskDelay(500);
    }
}

void anotherTask(void *pvPara)
{
    while (true)
    {
        if (W25N01::isBusy())
        {
            continue;
        }
        flash.ReadMemory(t_block, t_page, t_byte, buffer, 4);
        vTaskDelay(1);
    }
}

/**
 * @brief Create user tasks
 */
StackType_t uxBlinkTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xBlinkTaskTCB;
StackType_t uxAnotherTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xAnotherTaskTCB;
void startUserTasks()
{
    xTaskCreateStatic(blink, "blink", configMINIMAL_STACK_SIZE, NULL, 0, uxBlinkTaskStack, &xBlinkTaskTCB);
    xTaskCreateStatic(anotherTask, "anotherTask", configMINIMAL_STACK_SIZE, NULL, 0, uxAnotherTaskStack, &xAnotherTaskTCB);
}
