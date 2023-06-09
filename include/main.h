#ifndef _MAIN_H_
#define _MAIN_H_

#include "rgb_led.h"
#include "input.h"
#include "rgb_led.h"
#include "ultrasonic.h"
#include "ser_output.h"
#include "storage.h"
#if NTP_TIMESYNC
#include "time.h"

#define BOARD_ESP32_WROOM_IOT_v1 100

/**
 * @brief Pin definitions for the various sensors, actuators, and modules.
 */
#if BOARD == BOARD_ESP32_WROOM_IOT_v1
#define INCLUDE_MICRO_MOTION_DET

#ifdef INCLUDE_LDR
#define LDR 26 ///< Pin connected to the LDR sensor.
#endif         /* INCLUDE_LDR */

#define CON_SW 0     // 34        ///< Pin connected to the connection switch sensor.
#define LED_R 32     ///< Pin connected to the red LED.
#define LED_G 33     ///< Pin connected to the green LED.
#define LED_B 25     ///< Pin connected to the blue LED.
#define SER_CLK 13   ///< Pin connected to the shift register clock.
#define SER_LATCH 12 ///< Pin connected to the shift register latch.
#define SER_DATA 14  ///< Pin connected to the shift register data input.

#ifdef INCLUDE_MICRO_MOTION_DET
#define M_DET_OUT 35 ///< Motion Detection Pin
#endif               /* INCLUDE_MICRO_MOTION_DET */

#ifdef INCLUDE_ULTRASONIC
#define UL_TRIGG 26 ///< Pin connected to the ultrasonic sensor trigger.
#define UL_ECHO 27  ///< Pin connected to the ultrasonic sensor echo.
#endif              /* INCLUDE_ULTRASONIC*/

#else
#error "Select a Board"
#endif /* BOARD */

/**
 * @brief Global instances of various sensor and actuator objects.
 */
#ifdef INCLUDE_LDR
extern input g_ldr;         ///< Object representing the LDR sensor.
#endif                      /* INCLUDE_LDR */
extern input g_con_sw;      ///< Object representing the contact switch sensor.
extern rgb_led g_rgb;       ///< Object representing the RGB LED.
extern ser_output g_relays; ///< Object representing the shift register output.

#ifdef INCLUDE_ULTRASONIC
extern ultrasonic g_ultrasonic; ///< Object representing the ultrasonic sensor.
#endif                          /* INCLUDE_ULTRASONIC*/

#ifdef INCLUDE_MICRO_MOTION_DET
extern input g_motion_det;
#endif /* INCLUDE_MICRO_MOTION_DET */

extern struct tm g_timeinfo;
extern QueueHandle_t g_time_sync_q;
#endif /* NTP_TIMESYNC */

#define WLAN_TASK_PRIO configMAX_PRIORITIES - 2
#define SENSOR_TASK_PRIO configMAX_PRIORITIES - 5
#define TIME_TASK_PRIO configMAX_PRIORITIES - 6
#define CONTROL_TASK_PRIO configMAX_PRIORITIES - 10
#define ACTUATOR_TASK_PRIO configMAX_PRIORITIES - 10

#define TASK_DEFAULT_STACK 2048
#define TASK_WLAN_STACK 4096

#endif /*_MAIN_H_*/
