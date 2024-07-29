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
uint8_t buffer[64];
uint16_t t_byte = 0, t_page = 0, t_block = 0;
uint8_t hmm[64];
uint8_t start          = false;
int change             = 0;
long unsigned int bruh = 0;

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
    while (true)
    {
        if (W25N01::isBusy())
        {
            continue;
        }
        HAL_GPIO_TogglePin(LED_ACT_GPIO_Port, LED_ACT_Pin);
        HAL_GPIO_TogglePin(LASER_GPIO_Port, LASER_Pin);
        // const uint8_t *hmm = reinterpret_cast<const unsigned char *>("The current time is: ");

        vTaskDelay(500);
    }
}

void readTask(void *pvPara)
{
    while (true)
    {
        if (W25N01::isBusy())
        {
            continue;
        }

        flash.ReadMemory(t_block, t_page, t_byte, buffer, 64);
        vTaskDelay(1);
    }
}

void writeTask(void *pvPara)
{
    for (int i = 0; i < 64; i++)
    {
        hmm[i] = i;
    }

    while (W25N01::isBusy())
        ;
    while (true)
    {
        if (W25N01::isBusy())
        {
            continue;
        }
        flash.WriteMemory(t_block, hmm, 64);
        flash.getLast_ECC_page_failure(bruh);
        vTaskDelay(1);
    }
}

/**
 * @brief Create user tasks
 */
StackType_t uxBlinkTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xBlinkTaskTCB;
StackType_t uxReadTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xReadTaskTCB;
StackType_t uxWriteTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xWriteTaskTCB;
void startUserTasks()
{
    start = true;
    flash.init();
    flash.EraseChip();
    xTaskCreateStatic(blink, "blink", configMINIMAL_STACK_SIZE, NULL, 0, uxBlinkTaskStack, &xBlinkTaskTCB);
    xTaskCreateStatic(readTask, "readTask", configMINIMAL_STACK_SIZE, NULL, 0, uxReadTaskStack, &xReadTaskTCB);
    xTaskCreateStatic(writeTask, "writeTask", configMINIMAL_STACK_SIZE, NULL, 0, uxWriteTaskStack, &xWriteTaskTCB);
}
