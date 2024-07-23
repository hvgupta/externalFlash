/**
 * @file AppConfig_tempelate.h
 * @author my name (my@domain.com)
 * @brief Configuration file for the whole project
 * @version 0.1
 * @date 2023-06-21
 *
 * @copyright Copyright (c) 2023
 *
 */

/* @how to use this file
 * 1. Copy this file as `AppConfig.h`
 * 2. put it into your template directory
 * 3. modify the content of this file
 * 4. include this file in your project
 */

/* clang-format off */
#if 1 /*Set it to "1" to enable content*/

#ifndef APPCONFIG_H
#define APPCONFIG_H
#endif

#include "Config.h"


//1 for use, 0 for not use
/*====================
   DEBUG CONFIG
 *====================*/
/*if use debug, some debug watch variables could be seen*/
#define USE_DEBUG 0
    

/*====================
   IMU CONFIG
 *====================*/

/*Config which IMU you use BEGIN*/
#define USE_IMU 0
#if USE_IMU


#define USE_ADIS16470 0
#define USE_BMI088 0
#define USE_ICM42688 1
/*Config which IMU you use END*/

#if USE_ADIS16470
#define ADIS_SPI_DRIVER hspi1
// Chip select pin port
#define ADIS_CS_GPIO_Port ADIS_EN_GPIO_Port
// Chip select pin number
#define ADIS_CS_Pin ADIS_EN_Pin
#endif // USE_ADIS16470

#if USE_BMI088
#define BMI088_SPI hspi1
#endif // USE_BMI088

#endif // USE_IMU


/*Config which Maganetometer you use BEGIN*/
#define USE_MAG 0
#if USE_MAG

#define USE_LIS3MDL 1  // Select IMU Type
#endif // USE_MAG


#if USE_MAG || USE_IMU

#ifndef SENSOR_SPI

#if defined(STM32F407xx)
#define SENSOR_SPI hspi2
#elif defined(STM32G473xx)
#define SENSOR_SPI hspi1
#endif // defined (chips)

#endif // defined (SENSOR_SPI)

#endif // USE_MAG || USE_IMU





/*====================
   CHASSIS CONFIG
 *====================*/

#define USE_CHASSIS_CONTROLLER 0
#if USE_CHASSIS_CONTROLLER
    #define USE_OMNI_CHASSIS 0
    #define USE_MECANUM_CHASSIS 0
    #define ANGULAR_TO_LINEAR_RATIO 1.0f
    #define SLOWDOWN_INCREASE 1.0f
#endif

/*====================
   POWER CONTROLLER
 *====================*/
#define USE_POWER_CONTROLLER 0

/*===================
   SUPER CAPACITOR
 *===================*/
#define USE_SUPER_CAPACITOR 0
#define SUPER_CAPACITOR_CAN_INDEX 0 // 0 for CAN1, 1 for CAN2, 2 for CAN3

/*====================
   PID CONFIG
 *====================*/
#define PID_TIMEOUT_CUSTOM 0
#if PID_TIMEOUT_CUSTOM
    #define PID_DEFAULT_TIMEOUT pdMS_TO_TICKS(100)
#endif

#define PID_ALPHA_CUSTOM 0
#if PID_ALPHA_CUSTOM
    #define PID_DEFAULT_ALPHA 0.2f
#endif

#define PID_MAX_OUTPUT_CUSTOM 0
#if PID_MAX_OUTPUT_CUSTOM
    #define PID_DEFAULT_MAX_OUTPUT 10000.0fs
#endif

/*====================
   DJI MOTOR CONFIG
 *====================*/
#define USE_DJI_MOTOR 0
#if USE_DJI_MOTOR
    #define DJI_MOTOR_CAN1 1
    #define DJI_MOTOR_CAN2 1
    #define DJI_MOTOR_CAN3 1
    #define USE_DJI_MOTOR_TYPE_A 1
    #define USE_DJI_MOTOR_TYPE_B 0
#endif

/*====================
   DM4310 MOTOR CONFIG
 *====================*/
 #define USE_DM4310 0
 #if USE_DM4310
/*4310 id setting, must be the same as configrator*/
    #define DM4310_ID_CUSTOM 0
    #if DM4310_ID_CUSTOM
        #define MASTER_ID_START 0x300
        #define MASTER_ID_END 0x400
    #endif
    #define DM4310_UINT_CUSTOM 0
    #if DM4310_UINT_CUSTOM
        #define DEFAULT_P_MAX 12.5f
        #define DEFAULT_V_MAX 30.0f
        #define DEFAULT_T_MAX 10.0f
    #endif
    #define DM4310_MANAGER_INDEX 1
#endif

/*====================
   HT04 MOTOR CONFIG
 *====================*/
#define USE_HT04 0


/*====================
    LKMF MOTOR CONFIG
 *====================*/
#define USE_LKMFMotor 0

/*====================
   DMA SECTION CONFIG
 *====================*/
#define DMA_ATTRIBUTES_COSTUM 0
#if DMA_ATTRIBUTES_COSTUM
    #define DMA_BUFFER_ATTRIBUTE __attribute((used, section(".D1")))
#endif

/*====================
   DR16 CONFIG
 *====================*/
#define USE_DR16 0
#if USE_DR16
    #define USE_DR16_DMA 1
    #define USE_DR16_INTERRUPT 0
/*UART CONFIG*/
    #if defined(STM32F407xx)
    #define USE_RX_INV 0
    #define DR16_UART huart4
    #elif defined(STM32G473xx)
    #define USE_RX_INV 1
    #define DR16_UART huart1
    #endif // define Chips
#endif

/*====================
   FDCAN CONFIG
 *====================*/
#define USE_CAN_MANAGER 0
#if USE_CAN_MANAGER 
#define CAN_CUSTOM 1
#if CAN_CUSTOM
    #define CAN_NUM  3
    #define CAN_FILTER_NUM 16
    #define CAN_FILTER_SLAVE_START 14
    #define CAN_TX_QUEUE_LENGTH 5
#endif // CAN_CUSTOM
#endif // USE_CAN_MANAGER

/*====================
   Serial Interboard 
 *====================*/

#define ENABLE_SERIAL_INTERBOARD 0
// Note: You could not enable these two modules at the same time
#if ENABLE_SERIAL_INTERBOARD
/*Enable 1 master N slave interboard communication*/
/*Under Drivers/NewSerialInterComm/. */
#define USE_1TON_SERIAL_INTERBOARD 1
/*Enable Classical 1 master 1 slave interboard communication*/
/*Under Drivers/SerialInteroboard.* */
#define USE_SERIAL_INTERBOARD 0

#if USE_SERIAL_INTERBOARD
    #define INTERBOARD_MASTER 0
    #define INTERBOARD_SLAVE 1
    #define SERIAL_INTERBOARD_TASK_STACK_SIZE 256
    #define SERIAL_INTERBOARD_TX_BUFFER_SIZE 128
    #define SERIAL_INTERBOARD_RX_BUFFER_SIZE 256
    #define SERIAL_INTERBOARD_TX_QUEUE_LENGTH 8
    #define SERIAL_INTERBOARD_RX_TIMEOUT 5
    #define SERIAL_INTERBOARD_UART huart6
    #define SERIAL_INTERBOARD_DMA_BUFFER_ATTRIBUTE __attribute__((used, section(".D1")))
    // #define MAX_SERIAL_INTERBOARD_CONNECTIONS 

#elif USE_1TON_SERIAL_INTERBOARD
    #define SINGLE_FRAME_MODE 1 
    #if SINGLE_FRAME_MODE
        #define MILTI_FRAME_MODE 0
    #else
        #define MILTI_FRAME_MODE 1
    #endif
    #define INTERBOARD_MASTER 1 // Select master mode here
    #if INTERBOARD_MASTER
        #define INTERBOARD_SLAVE 0
        #define INTERBOARD_ADDRESS 0xFF
        #define SERIAL_INTERBOARD_CONNECTIONS_COUNT 2
        #define SLAVE_RESPONSE_TIMEOUT 280
    #else
        #define INTERBOARD_SLAVE 1 // Select Slave mode here
        #define INTERBOARD_ADDRESS 0x02
        #define SERIAL_INTERBOARD_CONNECTIONS_COUNT 1
    #endif
    #if defined(STM32F407xx)
        #define SERIAL_INTERBOARD_UART huart6
        #define SERIAL_INTERBOARD_TIM htim10
    #elif defined(STM32G473xx)
        #define SERIAL_INTERBOARD_UART huart4
        #define SERIAL_INTERBOARD_TIM htim16
    #endif
    #define MASTER_ADDRESS 0xFF
    #define SERIAL_INTERBOARD_TASK_STACK_SIZE 256
    #define SERIAL_INTERBOARD_TX_BUFFER_SIZE 128
    #define SERIAL_INTERBOARD_RX_BUFFER_SIZE 256
    #define SERIAL_INTERBOARD_TX_QUEUE_LENGTH 8
    #define SERIAL_INTERBOARD_RX_TIMEOUT 5
    #define SERIAL_INTERBOARD_DMA_BUFFER_ATTRIBUTE __attribute__((used, section(".D1")))
    #define SERIAL_INTERBOARD_DISCONNECT_LIMIT 5
#endif // Select Interboard module
#endif // ENALBE_SERIAL_INTERBOARD

/*====================
   REFEREE SYSTEM COMM
 *====================*/
#define USE_REFEREE_SYSTEM_COMM 0
#if USE_REFEREE_SYSTEM_COMM
#if defined(STM32F407xx)
    #define REFEREE_COMM_UART huart2
#elif defined(STM32G473xx)
    #define REFEREE_COMM_UART huart5
#endif // defined MCU
    #define REFEREE_SYS_COMM_SENDER_ROBOT_ID (uint16_t)ObjectID::RED_HERO // set your current robot ID
#endif // USE_REFEREE_SYSTEM_COMM 

/*====================
   ROS COMM CONFIG
 *====================*/
#define USE_ROS_COMM 0
#if USE_ROS_COMM
#define ROS_COMM_UART huart2
#define ROS_PROTOCOL_CUSTOM 0
#if ROS_PROTOCOL_CUSTOM
    #define ROS_COMM_16_BITS_DATA_LENGTH 0
    #define ROS_BUF_SIZE 32
    #define ROS_TX_QUEUE_LENGTH 16
    #define ROS_TX_TIMEOUT 10
    #define ROS_RX_TIMEOUT 50
    #define ROS_NUM_PROCESS_FUNCTION 32
#endif
#endif

/*====================
   KEY CONFIG
 *====================*/
#define USE_KEY 0
#if USE_KEY
    #define KEY_NUM 18
    #define EXTENDED_KEY 0
#endif

/*====================
   VTM COMM CONFIG
 *====================*/
#define USE_VTM_COMM 0
#if USE_VTM_COMM
    #define VTM_COMM_UART huart2
    #define VTM_MAX_BUFF_SIZE 80
    #define VTM_COMM_CUSTOMIZED_CONTROLLER_TIMEOUT pdMS_TO_TICKS(200)
    #define VTM_COMM_KEYBOARD_MOUSE_TIMEOUT pdMS_TO_TICKS(200)
#endif

/*====================
   BUZZER CONFIG
 *====================*/
#define USE_BUZZER 0

#if USE_BUZZER
    #define BUZZER_TIM htim9
    #define BUZZER_TIM_CHANNEL TIM_CHANNEL_2
    #define BUZZER_TIM_CLOCK 168000000.0f
    #define BUZZER_QUEUE_LENGTH 20
#endif

#define USE_FLASH 1
#endif // Content enable