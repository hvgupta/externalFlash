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

#include "config.h"

//1 for use, 0 for not use
/*====================
   DEBUG CONFIG
 *====================*/
/*if use debug, some debug watch variables could be seen*/
#define USE_DEBUG 1
    

/*====================
   IMU CONFIG
 *====================*/

/*Config which IMU you use BEGIN*/
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
#endif
#if USE_BMI088
#define BMI088_SPI hspi1
#endif
/*====================
   CHASSIS CONFIG
 *====================*/

#define USE_CHASSIS_CONTROLLER 0
#if USE_CHASSIS_CONTROLLER
    #define USE_OMNI_CHASSIS 0
    #define USE_MECANUM_CHASSIS 0
#endif

/*====================
   POWER CONTROLLER
 *====================*/
#define USE_POWER_CONTROLLER 0

/*====================
   SUPER CAPACITOR
 =====================*/
#define USE_SUPER_CAPACITOR 0

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
    #define USE_DJI_MOTOR_TYPE_A 1
    #define USE_DJI_MOTOR_TYPE_B 1
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
#endif

#define USE_LKMFMotor 0
#if USE_LKMFMotor
#define LKMFMotor_CAN_MANAGER_INDEX 0
#endif

/*====================
   DR16 CONFIG
 *====================*/
#define USE_DR16 0
#if USE_DR16
    #define USE_DR16_DMA 1
    #define USE_DR16_INTERRUPT 0
/*UART CONFIG*/
    #define DR16_UART huart4

/*DMA SECTION*/
#define DR16_DMA_ATTRIBUTES_COSTUM 0
#if DR16_DMA_ATTRIBUTES_COSTUM
    #define DR16_DMA_BUFFER_ATTRIBUTE __attribute((used, section(".D1")))
#endif
#endif

/*====================
   FDCAN CONFIG
 *====================*/
#define USE_CAN_MANAGER 0
#ifdef USE_CAN_MANAGER
#define CAN_CUSTOM 0
#if CAN_CUSTOM
    #define CAN_NUM 1
    #define CAN_FILTER_NUM 8
    #define CAN_FILTER_SLAVE_START 14
#endif
#endif

/*====================
   INTERBOARD COMM
 *====================*/
#define USE_INTERBOARD_COMM 0
#if USE_INTERBOARD_COMM
    #define INTERBOARD_COMM_CAN_INDEX 1
    #define INTERBOARD_COMM_MASTER 0
    #define INTERBOARD_CAN_LENGTH 8

        #if INTERBOARD_COMM_MASTER
            #define INTERBOARD_MASTER
        #else
            #define INTERBOARD_SLAVE
        #endif
#endif

#define USE_SERIAL_INTERBOARD 0
#if USE_SERIAL_INTERBOARD
    #define INTERBOARD_MASTER 0
    #define INTERBOARD_SLAVE 0
    #define INTERBOARD_COMM_UART huart2
    #define SERIAL_INTERBOARD_TASK_STACK_SIZE 256
    #define SERIAL_INTERBOARD_TX_BUFFER_SIZE 128
    #define SERIAL_INTERBOARD_RX_BUFFER_SIZE 256
    #define SERIAL_INTERBOARD_TX_QUEUE_LENGTH 8
    #define SERIAL_INTERBOARD_RX_TIMEOUT 5
    #define SERIAL_INTERBOARD_UART huart6
    #define SERIAL_INTERBOARD_DMA_BUFFER_ATTRIBUTE __attribute__((used, section(".D1")))
#endif

/*====================
   REFEREE SYSTEM COMM
 *====================*/
#define USE_REFEREE_SYSTEM_COMM 0
#if USE_REFEREE_SYSTEM_COMM
    #define REFEREE_BUFFER_SIZE 150
    #define REFEREE_RX_LIST_SIZE 64
    #define REFEREE_TX_MAX_BUFFER_SIZE 3
    #define REFEREE_COMM_UART huart2
#endif

/*====================
   ROS COMM CONFIG
 *====================*/
#define USE_ROS_COMM 0
#if USE_ROS_COMM
#define ROS_COMM_UART huart2
#define ROS_PROTOCOL_CUSTOM 0
#if ROS_PROTOCOL_CUSTOM
    #define ROS_BUF_SIZE 32
    #define ROS_TX_QUEUE_LENGTH 16
    #define ROS_TX_TIMEOUT 10
    #define ROS_RX_TIMEOUT 50
    #define ROS_NUM_PROCESS_FUNCTION 32
#endif
#endif
#endif