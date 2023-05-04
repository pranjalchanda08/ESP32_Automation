#include "mqtt.h"
#include "task_wifi_manager.h"
#include "task_control.h"

#define MQTT_TAG "MQTT"
#define SUB_MAX SUB_TOPIC_ID_MAX
#define PUB_MAX PUB_TOPIC_ID_MAX

#define ACTION_TOPIC(t) "dev9987/balcony/" #t

static char broker_name[20];

const char *subscription_list[SUB_MAX] = {
    ACTION_TOPIC(aut),
    ACTION_TOPIC(master),
    ACTION_TOPIC(ch0),
    ACTION_TOPIC(ch1),
    ACTION_TOPIC(ch2),
    ACTION_TOPIC(ch3),
    ACTION_TOPIC(WiFiSConfig),
};

const char *publish_list[PUB_MAX] = {
    ACTION_TOPIC(motion),
    ACTION_TOPIC(sunlight),
    ACTION_TOPIC(ultrasonic)};
AsyncMqttClient mqttClient;

void onMqttPublish(uint16_t packetId)
{
    ESP_LOGD(MQTT_TAG, "Publish acknowledged. packID: %d", packetId);
}

void onMqttConnect(bool sessionPresent)
{
    uint16_t packetIdSub;
    ESP_LOGI(MQTT_TAG, "Connected to MQTT.");
    for (int i = 0; i < SUB_MAX; i++)
    {
        packetIdSub = mqttClient.subscribe(subscription_list[i], 2);
        ESP_LOGD(MQTT_TAG, "Subscribing at QoS 2, packetId: %d", packetIdSub);
    }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    ESP_LOGE(MQTT_TAG, "Disconnected from MQTT.");

    if (WiFi.isConnected())
    {
        task_wlan_msg(WLAN_Q_MSG_ID_BEGIN_MQTT_CONNECT, NULL);
    }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    ESP_LOGD(MQTT_TAG, "Subscribe acknowledged. %d, qos: %d", packetId, qos);
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

    if (String(topic) == subscription_list[SUB_TOPIC_ID_CH_0])
    {
        state = messageTemp == "1";

        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_CH_0 %d", state);
        task_control_msg(CONTROL_Q_MSG_ID_SET_CH0_SWITCH, &state);
    }
    if (String(topic) == subscription_list[SUB_TOPIC_ID_CH_1])
    {
        state = messageTemp == "1";

        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_CH_1 %d", state);
        task_control_msg(CONTROL_Q_MSG_ID_SET_CH1_SWITCH, &state);
    }
    if (String(topic) == subscription_list[SUB_TOPIC_ID_CH_2])
    {
        state = messageTemp == "1";

        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_CH_2 %d", state);
        task_control_msg(CONTROL_Q_MSG_ID_SET_CH2_SWITCH, &state);
    }
    if (String(topic) == subscription_list[SUB_TOPIC_ID_CH_3])
    {
        state = messageTemp == "1";

        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_CH_3 %d", state);
        task_control_msg(CONTROL_Q_MSG_ID_SET_CH3_SWITCH, &state);
    }
    else if (String(topic) == subscription_list[SUB_TOPIC_ID_MASTER])
    {
        state = messageTemp == "1";
        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_MASTER %d", state);
        task_control_msg(CONTROL_Q_MSG_ID_SET_MASTER_SWITCH, &state);
    }
    else if (String(topic) == subscription_list[SUB_TOPIC_ID_AUT])
    {
        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_AUT %d", messageTemp == "1");
    }
    else if (String(topic) == subscription_list[SUB_TOPIC_ID_WIFI_SCONFIG])
    {
        ESP_LOGI(MQTT_TAG, "SUB_TOPIC_ID_WIFI_SCONFIG %d", messageTemp == "1");
        task_wlan_msg(WLAN_Q_MSG_ID_BEGIN_SCONFIG, NULL);
    }
}

void mqtt_client_init()
{
    memcpy(&broker_name, g_storage_struct.get_stored_broker().c_str(), g_storage_struct.get_stored_broker().length());
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setServer(broker_name, MQTT_PORT);
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

void mqtt_publish(pub_topic_id topic, const char *val, bool retain)
{
    mqttClient.publish(publish_list[topic], 2, retain, val);
}
