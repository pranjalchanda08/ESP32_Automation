#include <Arduino.h>
#include "task_actuator.h"
#include "task_sensor.h"
#include "task_control.h"
#include "task_wifi_manager.h"
#include "task_time.h"
#include "main.h"

#define SENSOR_TASK_PRIO 			configMAX_PRIORITIES
#define ACTUATOR_TASK_PRIO 			SENSOR_TASK_PRIO - 1
#define WLAN_TASK_PRIO 				SENSOR_TASK_PRIO - 5
#define CONTROL_TASK_PRIO 			SENSOR_TASK_PRIO - 10

#ifdef INCLUDE_LDR
input g_ldr = input(LDR);
#endif /*INCLUDE_LDR*/

input g_con_sw = input(CON_SW);
rgb_led g_rgb = rgb_led(LED_R, LED_G, LED_B);
ser_output g_relays = ser_output(SER_CLK, SER_LATCH, SER_DATA);
ultrasonic g_ultrasonic = ultrasonic(UL_TRIGG, UL_ECHO);

void setup()
{
	Serial.begin(115200);
	delay(1000);
	g_storage_struct.m_pref.begin("store_struct", false);
	esp_log_level_set("*", ESP_LOG_INFO);
	ESP_LOGI("main", "*********************************");
	ESP_LOGI("main", "* Project: Balcony Automation");
	ESP_LOGD("main", "* HEAP: %d", ESP.getHeapSize());
	ESP_LOGI("main", "* Initialising....");
	ESP_LOGI("main", "*********************************");
	xTaskCreate(task_control,
				"CONTROL_TASK",
				2048,
				NULL,
				CONTROL_TASK_PRIO,
				NULL);
	vTaskDelay(pdMS_TO_TICKS(100));
	xTaskCreate(task_sensor,
				"SENSOR_TASK",
				2048,
				NULL,
				SENSOR_TASK_PRIO,
				NULL);
	xTaskCreate(task_actuators,
				"ACTUATOR_TASK",
				2048,
				NULL,
				ACTUATOR_TASK_PRIO,
				NULL);
	xTaskCreate(task_wifi_manager,
				"WIFI_TASK",
				4096,
				NULL,
				WLAN_TASK_PRIO,
				NULL);
	vTaskDelay(pdMS_TO_TICKS(1000));
}

void loop()
{
}