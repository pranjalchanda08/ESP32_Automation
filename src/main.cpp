#include <Arduino.h>

#if 1

#include "task_actuator.h"
#include "task_sensor.h"
#include "task_control.h"
#include "task_wifi_manager.h"
#include "task_time.h"
#include "main.h"

#define WLAN_TASK_PRIO configMAX_PRIORITIES - 2
#define SENSOR_TASK_PRIO configMAX_PRIORITIES - 5
#define CONTROL_TASK_PRIO configMAX_PRIORITIES - 10
#define ACTUATOR_TASK_PRIO configMAX_PRIORITIES - 10

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
	delay(5000);
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

#else

#include <WiFi.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <AsyncMqttClient.h>

#define WIFI_SSID "Jaguar"
#define WIFI_PASSWORD "ncaa7708"

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883

AsyncMqttClient mq_client;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

void connect_wifi()
{
	Serial.println("Connecting to Wi-Fi...");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connect_mqtt()
{
	Serial.println("Connecting to MQTT...");
	mq_client.connect();
}

void wifi_event(WiFiEvent_t event)
{
	Serial.printf("[WiFi-event] event: %d\n", event);
	switch (event)
	{
	case SYSTEM_EVENT_STA_GOT_IP:
		Serial.println("WiFi connected");
		Serial.println("IP address: ");
		Serial.println(WiFi.localIP());
		connect_mqtt();
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		Serial.println("WiFi lost connection");
		xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
		xTimerStart(wifiReconnectTimer, 0);
		break;
	}
}

void on_mq_con(bool sessionPresent)
{
	Serial.println("Connected to MQTT.");
	Serial.print("Session present: ");
	Serial.println(sessionPresent);
	uint16_t packetIdSub = mq_client.subscribe("balcony/light", 2);
	Serial.print("Subscribing at QoS 2, packetId: ");
	packetIdSub = mq_client.subscribe("balcony/aut", 2);
	Serial.print("Subscribing at QoS 2, packetId: ");
	packetIdSub = mq_client.subscribe("balcony/fountain", 2);
	Serial.print("Subscribing at QoS 2, packetId: ");

	//   Serial.println(packetIdSub);
	//   mq_client.publish("test/", 0, true, "test 1");
	//   Serial.println("Publishing at QoS 0");
	//   uint16_t packetIdPub1 = mq_client.publish("test/lol", 1, true, "test 2");
	//   Serial.print("Publishing at QoS 1, packetId: ");
	//   Serial.println(packetIdPub1);
	//   uint16_t packetIdPub2 = mq_client.publish("test/lol", 2, true, "test 3");
	//   Serial.print("Publishing at QoS 2, packetId: ");
	//   Serial.println(packetIdPub2);
}

void on_mq_discon(AsyncMqttClientDisconnectReason reason)
{
	Serial.println("Disconnected from MQTT.");

	if (WiFi.isConnected())
	{
		xTimerStart(mqttReconnectTimer, 0);
	}
}

void on_mq_sub(uint16_t packetId, uint8_t qos)
{
	Serial.println("Subscribe acknowledged.");
	Serial.print("  packetId: ");
	Serial.println(packetId);
	Serial.print("  qos: ");
	Serial.println(qos);
}

void on_mq_unsub(uint16_t packetId)
{
	Serial.println("Unsubscribe acknowledged.");
	Serial.print("  packetId: ");
	Serial.println(packetId);
}

void on_msg(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
	Serial.println("Publish received.");
	Serial.print("  topic: ");
	Serial.println(topic);
	Serial.print("  qos: ");
	Serial.println(properties.qos);
	Serial.print("  dup: ");
	Serial.println(properties.dup);
	Serial.print("  retain: ");
	Serial.println(properties.retain);
	Serial.print("  len: ");
	Serial.println(len);
	Serial.print("  index: ");
	Serial.println(index);
	Serial.print("  total: ");
	Serial.println(total);
}

void on_pub(uint16_t packetId)
{
	Serial.println("Publish acknowledged.");
	Serial.print("  packetId: ");
	Serial.println(packetId);
}

void setup()
{
	Serial.begin(115200);
	Serial.println();
	Serial.println();

	mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connect_mqtt));
	wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connect_wifi));

	WiFi.onEvent(wifi_event);

	mq_client.onConnect(on_mq_con);
	mq_client.onDisconnect(on_mq_discon);
	mq_client.onSubscribe(on_mq_sub);
	mq_client.onUnsubscribe(on_mq_unsub);
	mq_client.onMessage(on_msg);
	mq_client.onPublish(on_pub);
	mq_client.setServer(MQTT_HOST, MQTT_PORT);

	connect_wifi();
}

void loop()
{
}

#endif