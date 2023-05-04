#ifndef _MQTT_H_
#define _MQTT_H_

#include "main.h"
#include "storage.h"

#include <WiFi.h>
#include <AsyncMqttClient.h>

#define MQTT_PORT 1883

typedef enum
{
    SUB_TOPIC_ID_AUT = 0,
    SUB_TOPIC_ID_MASTER,
    SUB_TOPIC_ID_CH_0,
    SUB_TOPIC_ID_CH_1,
    SUB_TOPIC_ID_CH_2,
    SUB_TOPIC_ID_CH_3,
    SUB_TOPIC_ID_WIFI_SCONFIG,
    SUB_TOPIC_ID_MAX
} sub_topic_id;

typedef enum
{
    PUB_TOPIC_ID_MOTION = 0,
    PUB_TOPIC_ID_SUNLIGHT,
    PUB_TOPIC_ID_ULTRASONIC,
    PUB_TOPIC_ID_MAX
} pub_topic_id;

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void mqtt_publish(pub_topic_id topic, const char* val, bool retain);
void mqtt_client_init();
void mqtt_reconnect();

#endif /* _MQTT_H_ */