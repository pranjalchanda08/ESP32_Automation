/**
 * @file task_wifi_manager.cpp
 * @brief Task to handle WiFi connectivity.
 */

#include "task_wifi_manager.h"
#include "task_control.h"
#include "task_time.h"
#include "WiFi.h"

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
static bool connectWiFiUsingSconfig();

#else

#define DEFAULT_SSID    g_storage_struct.get_stored_SSID()  /**< Default SSID. */
#define DEFAULT_PSK     g_storage_struct.get_stored_PSK()   /**< Default PSK. */

#endif /*WLAN_SCONFIG*/

QueueHandle_t g_wlan_q;
#define WLAN_QUEUE_LEN 10                    /**< Length of the WiFi manager queue. */
#define WLAN_QUEUE_SIZE sizeof(wlan_q_msg_t) /**< Size of each element in the WiFi manager queue. */

static wlan_struct_t s_wlan_struct; /**< Structure to hold the WiFi connection state. */

void task_wifi_manager(void *args)
{
    WiFi.mode(WIFI_STA);
    uint32_t time_q_msg_rx = false;
    g_wlan_q = xQueueCreate(WLAN_QUEUE_LEN, WLAN_QUEUE_SIZE);
    g_time_sync_q = xQueueCreate(10, sizeof(uint32_t));
    wlan_q_msg_t msg;
    uint8_t retry;
    String SSID;
    String PSK;

    ESP_LOGI("wifi", "task_wifi_manager created");

    xTaskCreate(task_time,
                "TIME_TASK",
                2096,
                NULL,
                19,
                NULL);
    while (true)
    {
        if (xQueueReceive(g_wlan_q, &msg, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            switch (msg.wlan_q_msg_id)
            {
            case WLAN_Q_MSG_ID_BEGIN_SCONFIG:
#if WLAN_SCONFIG
                ESP_LOGI("wifi", "Starting Smart Config");
                connectWiFiUsingSconfig();
#else
                ESP_LOGE("wifi", "WLAN_SCONFIG NOT Enabled");
#endif /* WLAN_SCONFIG */
                break;

            default:
                break;
            }
        }
        else
        {
            if (!WiFi.isConnected())
            {
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
                    if(PSK.equals("DEADBEEF"))
                    {
                        ESP_LOGE("wifi", "PSK read failed from nvm");
                    }
                    else
                    {
                        ESP_LOGD("wifi", "connecting to SSID: %s, PSK: %s", SSID, PSK);
                        WiFi.begin(SSID.c_str(), PSK.c_str());
                        retry = 20;
                        while (WiFi.status() != WL_CONNECTED && retry)
                        {
                            g_rgb << "0000FF"; /* Blue */
                            retry--;
                            vTaskDelay(pdMS_TO_TICKS(500));
                        }
                        if (WiFi.status() == WL_CONNECTED)
                        {
                            ESP_LOGI("wifi", "WLAN Connected! SSID:%s IP: %s", WiFi.SSID(), WiFi.localIP().toString().c_str());
                            s_wlan_struct.wlan_connect_status = true;
                            g_rgb << "09BA09"; /* Green Shade */
                            
                            /* Unlock timesync Thread to sync */
                            time_q_msg_rx = true;
                            xQueueSend(g_time_sync_q, &time_q_msg_rx, portMAX_DELAY);
                        }
                        else
                        {
                            ESP_LOGE("wifi", "WLAN Connected failed! Retry: %d", s_wlan_struct.wlan_connect_retry);
                        }
                    }
                }
                s_wlan_struct.wlan_connect_retry++;
            }
            else
            {
                s_wlan_struct.wlan_connect_status = true;
                g_rgb << "09BA09"; /* Green Shade */
            }
        }
    }
}

BaseType_t task_wlan_msg(wlan_q_msg_id_t msg_id, void *p_payload)
{
    static wlan_q_msg_t msg;

    msg.wlan_q_msg_id = msg_id;
    msg.p_payload = p_payload;

    return xQueueSend(g_wlan_q, &msg, portMAX_DELAY);
}

BaseType_t task_wlan_msg_isr(wlan_q_msg_id_t msg_id, void *p_payload)
{
    static wlan_q_msg_t msg;

    BaseType_t yeild = pdFALSE;
    msg.wlan_q_msg_id = msg_id;
    msg.p_payload = p_payload;

    return xQueueSendFromISR(g_wlan_q, &msg, &yeild);
}

#if WLAN_SCONFIG
static bool connectWiFiUsingSconfig()
{
    int status = false;
    int retry = 100;
    uint32_t rgb_state = 0x000000FF;

    String SSID = "DEADBEEF";
    String PSK = "DEAFBEEF";

    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();

    while (!WiFi.smartConfigDone() && retry)
    {
        rgb_state = ((rgb_state ^ 0x000000FF) && 0x000000FF);
        g_rgb << rgb_state;
        retry--;
        vTaskDelay(500 / portTICK_RATE_MS);
        ESP_LOGW("wifi","Waiting for smart config packets: Retry left: %d", retry);
    }
    if (!retry)
    {
        WiFi.stopSmartConfig();
        ESP_LOGE("wifi", "No Packets from Smart Config");
    }
    else
    {
        retry = 10;
        while (WiFi.status() != WL_CONNECTED && retry)
        {
            retry--;
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        if (!retry)
        {
            ESP_LOGE("wifi", "WLAN Connect Timeout");
        }
        else
        {
            ESP_LOGI("wifi", "Connected to SSID: %s, IP: %s",
                     WiFi.SSID(),
                     WiFi.localIP());
            status = true;

            SSID = WiFi.SSID();
            PSK = WiFi.psk();

            g_storage_struct.store_SSID(SSID);
            g_storage_struct.store_PSK(PSK);
            ESP_LOGD("wifi", "SSID, PASS saved in NVM");
        }
    }
    WiFi.mode(WIFI_STA);
    return status;
}

#endif /* WLAN_SCONFIG */