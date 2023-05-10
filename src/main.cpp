#include <Arduino.h>

#include "task_actuator.h"
#include "task_sensor.h"
#include "task_control.h"
#include "task_wifi_manager.h"
#include "task_time.h"
#include "main.h"

#define TAG_MAIN "MAIN"

#ifdef INCLUDE_LDR
input g_ldr = input(LDR);
#endif /*INCLUDE_LDR*/

input g_con_sw = input(CON_SW);
rgb_led g_rgb = rgb_led(LED_R, LED_G, LED_B);
ser_output g_relays = ser_output(SER_CLK, SER_LATCH, SER_DATA);

#ifdef INCLUDE_ULTRASONIC
ultrasonic g_ultrasonic = ultrasonic(UL_TRIGG, UL_ECHO);
#endif /* INCLUDE_ULTRASONIC*/

#ifdef INCLUDE_MICRO_MOTION_DET
input g_motion_det = input(M_DET_OUT);
#endif /* INCLUDE_MICRO_MOTION_DET */

void setup()
{
	Serial.begin(115200);
	Serial.println();
	g_storage_struct.m_pref.begin("store_struct", false);

	ESP_LOGI(TAG_MAIN, "*********************************");
	ESP_LOGI(TAG_MAIN, "* Project: Balcony Automation");
	ESP_LOGD(TAG_MAIN, "* HEAP: %d", ESP.getHeapSize());
	ESP_LOGI(TAG_MAIN, "* Initialising....");
	ESP_LOGI(TAG_MAIN, "*********************************");

    /* Create Alarm Task */
    xTaskCreate(task_time, "TIME_TASK", TASK_DEFAULT_STACK, NULL, TIME_TASK_PRIO, NULL);
    /* Create Sensor Task */
	xTaskCreate(task_sensor, "SENSOR_TASK", TASK_DEFAULT_STACK, NULL, SENSOR_TASK_PRIO, NULL);
    /* Create Control Task */
	xTaskCreate(task_control, "CONTROL_TASK", TASK_DEFAULT_STACK, NULL, CONTROL_TASK_PRIO, NULL);
    /* Create Actuator Task Task */
	xTaskCreate(task_actuators, "ACTUATOR_TASK", TASK_DEFAULT_STACK, NULL, ACTUATOR_TASK_PRIO, NULL);
    /* Create WiFi Manager Task */
	xTaskCreate(task_wifi_manager, "WIFI_TASK", TASK_WLAN_STACK, NULL, WLAN_TASK_PRIO, NULL);
}

void loop()
{
}