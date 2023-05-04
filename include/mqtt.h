#ifndef _MQTT_H_
#define _MQTT_H_

#include "main.h"

#include <WiFi.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#include <AsyncMqttClient.h>


#define MQTT_HOST "broker.hivemq.com"//IPAddress(192, 168, 1, 10)
#define MQTT_PORT 1883

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void mqtt_publish(const char* topic, const char* val, bool retain);
void mqtt_client_init();
void mqtt_reconnect();
bool mqtt_is_conn();

#endif /* _MQTT_H_ */