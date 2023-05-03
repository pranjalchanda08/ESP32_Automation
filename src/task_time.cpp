#include "task_time.h"
#include "task_wifi_manager.h"
#include "task_control.h"
#include "WiFi.h"

#define TIME_MIN(hr, min) ((hr * 60) + min)

#define SUNRISE_HR 7
#define SUNRISE_MIN 0

#define SUNSET_HR 17
#define SUNSET_MIN 56

/* India Time Zone */
static const char *s_ntpServer = "in.pool.ntp.org";
static const long s_gmtOffset_sec = 19800;
static const int s_daylightOffset_sec = 0;

void task_time(void *args)
{
    struct tm now;
    uint32_t msg_rx = false;
    bool sunlight = false;
    while (true)
    {
        uint32_t msg;
        if (pdTRUE == xQueueReceive(g_time_sync_q, &msg, pdMS_TO_TICKS(1)))
        {
#if NTP_TIMESYNC
            if (msg == true)
            {
                configTime(s_gmtOffset_sec, s_daylightOffset_sec, s_ntpServer);
                if (!getLocalTime(&now))
                {
                    ESP_LOGE("wifi", "Failed to obtain time");
                    msg_rx = true;
                    xQueueSend(g_time_sync_q, &msg_rx, portMAX_DELAY);
                }
                else
                {
                    ESP_LOGI("wifi", "Time Synced: %d:%d:%d",
                             now.tm_hour,
                             now.tm_min,
                             now.tm_sec);
                }
            }
            else
            {
                getLocalTime(&now);
                long now_min = TIME_MIN(now.tm_hour, now.tm_min);
                long sunrise_min = TIME_MIN(SUNRISE_HR, SUNRISE_MIN);
                long sunset_min = TIME_MIN(SUNSET_HR, SUNSET_MIN);

                if ((now_min < sunrise_min) || (now_min >= sunset_min))
                {
                    /* Lights On */
                    ESP_LOGD("task_time", "LIGHTS ON");
                    sunlight = false;
                }
                else
                {
                    /* Lights Off */
                    ESP_LOGD("task_time", "LIGHTS OFF");
                    sunlight = true;
                }
                task_control_msg(CONTROL_Q_MSG_ID_SET_SUNLIGHT_DETECTED, &sunlight);
            }

#endif /* NTP_TIMESYNC */
        }
        else
        {
            /* Message to self to perform a time get */
            if(g_time_sync_q != NULL)
            {
                msg_rx = false;
                xQueueSend(g_time_sync_q, &msg_rx, portMAX_DELAY);
            }
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }
}