#include "mqtt.h"
#include "task_wifi_manager.h"
#include "task_control.h"

#define MQTT_TAG "MQTT"
#define SUB_MAX SUB_TOPIC_ID_MAX

typedef enum
{
    SUB_TOPIC_ID_AUT = 0,
    SUB_TOPIC_ID_MASTER,
    SUB_TOPIC_ID_FOUNTAIN,
    SUB_TOPIC_ID_LIGHT,
    SUB_TOPIC_ID_MAX
} sub_topic_id;

const char *subscription_list[SUB_MAX] = {
    "dev9987/balcony/aut",
    "dev9987/balcony/master",
    "dev9987/balcony/fountain",
    "dev9987/balcony/light"
};

AsyncMqttClient mqttClient;

void onMqttPublish(uint16_t packetId)
{
    ESP_LOGI(MQTT_TAG, "Publish acknowledged. packID: %d", packetId);
}

void onMqttConnect(bool sessionPresent)
{
    uint16_t packetIdSub;
    ESP_LOGI(MQTT_TAG, "Connected to MQTT.");
    for (int i = 0; i < SUB_MAX; i++)
    {
        packetIdSub = mqttClient.subscribe(subscription_list[i], 2);
        ESP_LOGI(MQTT_TAG, "Subscribing at QoS 2, packetId: %d", packetIdSub);
    }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    ESP_LOGW(MQTT_TAG, "Disconnected from MQTT.");

    if (WiFi.isConnected())
    {
        /*TODO*/
        task_wlan_msg(WLAN_Q_MSG_ID_BEGIN_MQTT_CONNECT, NULL);
    }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    ESP_LOGI(MQTT_TAG, "Subscribe acknowledged. %d, qos: %d", packetId, qos);
}

void onMqttUnsubscribe(uint16_t packetId)
{
    ESP_LOGW(MQTT_TAG, "unsubscribed packet: %d", packetId);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    ESP_LOGD(MQTT_TAG, "Message arrived on topic: %s", topic);
    String messageTemp;
    bool state;

    for (int i = 0; i < len; i++)
    {
        messageTemp += (char)payload[i];
    }
    ESP_LOGD(MQTT_TAG, "Message: %s", messageTemp);

    if (String(topic) == subscription_list[SUB_TOPIC_ID_LIGHT])
    {
        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_LIGHT %d", messageTemp == "1");
    }
    else if (String(topic) == subscription_list[SUB_TOPIC_ID_MASTER])
    {
        state = messageTemp == "1";
        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_MASTER %d", state);
        task_control_msg(CONTROL_Q_MSG_ID_SET_MASTER_SWITCH, &state);
    }
    else if (String(topic) == subscription_list[SUB_TOPIC_ID_FOUNTAIN])
    {
        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_FOUNTAIN %d", messageTemp == "1");
    }
    else if (String(topic) == subscription_list[SUB_TOPIC_ID_AUT])
    {
        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_AUT %d", messageTemp == "1");
    }
}

void mqtt_client_init()
{
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    ESP_LOGI(MQTT_TAG, "mqtt_client_init Done");
}

void mqtt_reconnect()
{
    // Loop until we're reconnected
    mqttClient.connect();
}

bool mqtt_is_conn()
{
    return mqttClient.connected();
}

void mqtt_publish(const char* topic, const char* val, bool retain)
{
    mqttClient.publish(topic, 2, retain, val);
}
