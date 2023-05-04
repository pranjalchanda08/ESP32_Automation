/**
 * @file task_wifi_manager.cpp
 * @brief Task to handle WiFi connectivity.
 */

#include "task_wifi_manager.h"
#include "task_control.h"
#include "task_time.h"
#include "WiFi.h"
#include "mqtt.h"

#if NTP_TIMESYNC
#include "time.h"

QueueHandle_t g_time_sync_q = NULL;

#endif /* NTP_TIMESYNC */

#if WLAN_SCONFIG
/**
 * @brief Connects to a WiFi network using Smart Config.
 *
 * @return `true` if the connection is successful, `false` otherwise.
 */
static bool connect_wlan_using_sconfig();

#else

#define DEFAULT_SSID g_storage_struct.get_stored_SSID() /**< Default SSID. */
#define DEFAULT_PSK g_storage_struct.get_stored_PSK()   /**< Default PSK. */

#endif /*WLAN_SCONFIG*/
/**
 * @brief Connects to a WiFi network manually
 *
 * @return `true` if the connection is successful, `false` otherwise.
 */
static bool wlan_man_connect();

QueueHandle_t g_wlan_q;
#define WLAN_QUEUE_LEN 10                    /**< Length of the WiFi manager queue. */
#define WLAN_QUEUE_SIZE sizeof(wlan_q_msg_t) /**< Size of each element in the WiFi manager queue. */

static wlan_struct_t s_wlan_struct; /**< Structure to hold the WiFi connection state. */

void WiFiEvent(WiFiEvent_t event)
{
    uint32_t time_q_msg_rx = false;
    ESP_LOGD("WiFi-event", "%d\n", event);
    switch (event)
    {
    case WIFI_EVENT_STA_START:
        s_wlan_struct.wlan_connect_status = true;
        task_wlan_msg(WLAN_Q_MSG_ID_BEGIN_WLAN_CONNECT, NULL);
        s_wlan_struct.wlan_connect_status = false;
        
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI("WiFi-event", "WLAN Connected! SSID:%s IP: %s", WiFi.SSID(), WiFi.localIP().toString().c_str());
        s_wlan_struct.wlan_connect_status = true;
        g_rgb << "09BA09"; /* Green Shade */

        /* Unlock timesync Thread to sync */
        time_q_msg_rx = true;
        xQueueSend(g_time_sync_q, &time_q_msg_rx, portMAX_DELAY);
        task_wlan_msg(WLAN_Q_MSG_ID_BEGIN_MQTT_CONNECT, NULL);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        s_wlan_struct.wlan_connect_status = false;
        ESP_LOGI("WiFi-event", "WiFi lost connection");
        /* Dont send connect command from here this will hamper sconfig */
        break;
    }
}

void task_wifi_manager(void *args)
{
    WiFi.mode(WIFI_STA);
    WiFi.onEvent(WiFiEvent);

    g_wlan_q = xQueueCreate(WLAN_QUEUE_LEN, WLAN_QUEUE_SIZE);
    g_time_sync_q = xQueueCreate(10, sizeof(uint32_t));
    wlan_q_msg_t msg;

    ESP_LOGI("wifi", "task_wifi_manager created");

    /* Create Alarm Task */
    xTaskCreate(task_time, "TIME_TASK", 2096, NULL, 19, NULL);

    mqtt_client_init();

    while (true)
    {
        if (xQueueReceive(g_wlan_q, &msg, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            switch (msg.wlan_q_msg_id)
            {
            case WLAN_Q_MSG_ID_BEGIN_SCONFIG:
#if WLAN_SCONFIG
                ESP_LOGI("wifi", "Starting Smart Config");
                if (connect_wlan_using_sconfig() == true)
                {
                    g_storage_struct.store_SSID(WiFi.SSID());
                    g_storage_struct.store_PSK(WiFi.psk());
                    ESP_LOGD("wifi", "SSID, PASS saved in NVM");
                }
                else
                {

                }
#else
                ESP_LOGE("wifi", "WLAN_SCONFIG NOT Enabled");
#endif /* WLAN_SCONFIG */
                break;
            case WLAN_Q_MSG_ID_BEGIN_WLAN_CONNECT:
                s_wlan_struct.wlan_connect_in_progress = true;
                wlan_man_connect();
                break;
            case WLAN_Q_MSG_ID_BEGIN_MQTT_CONNECT:
                mqtt_reconnect();
                break;
            default:
                break;
            }
        }
        else
        {
            if (s_wlan_struct.wlan_connect_status == false && s_wlan_struct.wlan_connect_in_progress == false)
            {
                /* Disconnected */
                task_wlan_msg(WLAN_Q_MSG_ID_BEGIN_WLAN_CONNECT, NULL);
            }
            /* MQTT Internal calls */
            mqtt_loop();
        }
    }
}

BaseType_t task_wlan_msg(wlan_q_msg_id_t msg_id, void *p_payload)
{
    static wlan_q_msg_t msg;
    BaseType_t status = pdFALSE;

    msg.wlan_q_msg_id = msg_id;
    msg.p_payload = p_payload;

    status = xQueueSend(g_wlan_q, &msg, portMAX_DELAY);
    if (status == pdFALSE)
    {
        ESP_LOGE("wifi", "Queue Send Failure");
    }
    return status;
}

BaseType_t task_wlan_msg_isr(wlan_q_msg_id_t msg_id, void *p_payload)
{
    static wlan_q_msg_t msg;

    BaseType_t yeild = pdFALSE;
    msg.wlan_q_msg_id = msg_id;
    msg.p_payload = p_payload;

    return xQueueSendFromISR(g_wlan_q, &msg, &yeild);
}

static bool wlan_man_connect()
{
    uint8_t retry;
    String SSID;
    String PSK;

    s_wlan_struct.wlan_connect_status = false;
    g_rgb << "FF0D6E"; /* Red */
#if WLAN_SCONFIG
    SSID = g_storage_struct.get_stored_SSID();
#else
    SSID = DEFAULT_SSID;
#endif /* WLAN_SCONFIG */
    if (SSID.length() < 1 || SSID.equals("DEADBEEF"))
    {
        ESP_LOGE("wifi", "SSID not saved in NVM");
    }
    else
    {
        ESP_LOGI("wifi", "Connecting to SSID: %s", SSID.c_str());
#if WLAN_SCONFIG
        PSK = g_storage_struct.get_stored_PSK();
#else
        PSK = DEFAULT_PSK;
#endif /* WLAN_SCONFIG */
        if (PSK.equals("DEADBEEF"))
        {
            ESP_LOGE("wifi", "PSK read failed from nvm");
        }
        else
        {
            ESP_LOGD("wifi", "connecting to SSID: %s, PSK: %s", SSID, PSK);
            WiFi.begin(SSID.c_str(), PSK.c_str());
            retry = 50;
            while (WiFi.status() != WL_CONNECTED && retry)
            {
                g_rgb << "0000FF"; /* Blue */
                retry--;
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            if (WiFi.status() == WL_CONNECTED)
            {
            }
            else
            {
                ESP_LOGE("wifi", "WLAN Connection failed! Retry: %d", s_wlan_struct.wlan_connect_retry);
            }
        }
    }
    s_wlan_struct.wlan_connect_retry++;
    return true;
}

#if WLAN_SCONFIG
static bool connect_wlan_using_sconfig()
{
    int status = false;
    int retry = 100;
    bool b_state = true;

    String SSID = "DEADBEEF";
    String PSK = "DEAFBEEF";

    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();

    while (!WiFi.smartConfigDone() && retry)
    {
        b_state ^= true;
        g_rgb.set("b", b_state);
        retry--;
        vTaskDelay(500 / portTICK_RATE_MS);
        ESP_LOGW("wifi", "Waiting for smart config packets: Retry left: %d", retry);
    }
    if (!retry)
    {
        WiFi.stopSmartConfig();
        ESP_LOGE("wifi", "No Packets from Smart Config");
        status = false;
    }
    else
    {   
        status = true;
        retry = 10;
        while (WiFi.status() != WL_CONNECTED && retry)
        {
            retry--;
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        if (!retry)
        {
            ESP_LOGE("wifi", "WLAN Connect Timeout");
            status = false;
        }
    }
    WiFi.mode(WIFI_STA);
    return status;
}

#endif /* WLAN_SCONFIG */