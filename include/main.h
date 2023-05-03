#ifndef _MAIN_H_
#define _MAIN_H_

#include "rgb_led.h"
#include "input.h"
#include "rgb_led.h"
#include "ultrasonic.h"
#include "ser_output.h"
#include "storage.h"

#define WLAN_SCONFIG 1
#define NTP_TIMESYNC 1

/**
 * @brief Pin definitions for the various sensors, actuators, and modules.
 */
#define LDR         36      ///< Pin connected to the LDR sensor.
#define CON_SW      0       ///< Pin connected to the connection switch sensor.
#define LED_R       32      ///< Pin connected to the red LED.
#define LED_G       33      ///< Pin connected to the green LED.
#define LED_B       25      ///< Pin connected to the blue LED.
#define SER_CLK     13      ///< Pin connected to the shift register clock.
#define SER_LATCH   12      ///< Pin connected to the shift register latch.
#define SER_DATA    14      ///< Pin connected to the shift register data input.
#define UL_TRIGG    26      ///< Pin connected to the ultrasonic sensor trigger.
#define UL_ECHO     27      ///< Pin connected to the ultrasonic sensor echo.

/**
 * @brief Global instances of various sensor and actuator objects.
 */
#ifdef INCLUDE_LDR
extern input g_ldr;               ///< Object representing the LDR sensor.
#endif /* INCLUDE_LDR */
extern input g_con_sw;            ///< Object representing the contact switch sensor.
extern rgb_led g_rgb;             ///< Object representing the RGB LED.
extern ser_output g_relays;       ///< Object representing the shift register output.
extern ultrasonic g_ultrasonic;   ///< Object representing the ultrasonic sensor.

#if NTP_TIMESYNC
#include "time.h"

extern struct tm g_timeinfo;
extern QueueHandle_t g_time_sync_q;
#endif /* NTP_TIMESYNC */

#endif  /*_MAIN_H_*/
