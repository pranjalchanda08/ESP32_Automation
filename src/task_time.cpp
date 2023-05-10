#include "task_time.h"
#include "task_wifi_manager.h"
#include "task_control.h"
#include "WiFi.h"

#define LOG_TAG_TASK_TIME       "TASK_TIME"

#define TIME_MIN(hr, min) ((hr * 60) + min)

#define SUNRISE_HR 7
#define SUNRISE_MIN 0

#define SUNSET_HR 18
#define SUNSET_MIN 30

/* India Time Zone */
static const char *s_ntpServer = CONFIG_ESP_RMAKER_SNTP_SERVER_NAME;
static const long s_gmtOffset_sec = 19800;
static const int s_daylightOffset_sec = 0;

static c_alarm *alarm_reg[ALARM_TIMER_ID_MAX];

c_alarm::c_alarm(alarm_timer_id tim_id, struct tm start_time, struct tm alarm_offset, bool reload, alarm_cb alarm_cb)
{
    struct tm alarm;
    this->alarm_offset = alarm_offset;
    this->tim_id = tim_id;
    alarm = start_time;
    alarm.tm_hour += alarm_offset.tm_hour;
    alarm.tm_min += alarm_offset.tm_min;
    alarm.tm_sec += alarm_offset.tm_sec;
    this->alarm_time = mktime(&alarm); // Save as epoc time
    this->reload = reload;
    this->m_alarm_cb = alarm_cb;
    this->armed = true;
    alarm_reg[tim_id] = this;
}

c_alarm::c_alarm(alarm_timer_id tim_id, struct tm alarm_offset, bool reload, alarm_cb alarm_cb)
{
    struct tm alarm;
    struct tm start_time;
    time_t t_now;

    time(&t_now);
    localtime_r(&t_now, &start_time);

    this->alarm_offset = alarm_offset;
    this->tim_id = tim_id;
    alarm = start_time;
    alarm.tm_hour += alarm_offset.tm_hour;
    alarm.tm_min += alarm_offset.tm_min;
    alarm.tm_sec += alarm_offset.tm_sec;
    this->alarm_time = mktime(&alarm); // Save as epoc time
    this->reload = reload;
    this->m_alarm_cb = alarm_cb;
    this->armed = true;
    alarm_reg[tim_id] = this;
}

c_alarm::~c_alarm()
{
}

void tim_0_alarm_cb(void *args)
{
    ESP_LOGV(LOG_TAG_TASK_TIME, "tim_0_alarm_cb");
    if (g_time_sync_q != NULL)
    {
        task_time_msg(false);
    }
}

void task_time(void *args)
{
    uint32_t msg;
    uint32_t msg_rx = false;
    static bool time_synced = false;
    bool sunlight = false;

    uint8_t tim_itr_cntr = 0;
    uint32_t now_epoc;
    struct tm now;
    time_t t_now;
    struct tm tim_0_offset;

    tim_0_offset.tm_hour = 0;
    tim_0_offset.tm_min = 1;
    tim_0_offset.tm_sec = 0;

    g_time_sync_q = xQueueCreate(10, sizeof(uint32_t));

    while (true)
    {
        if (pdTRUE == xQueueReceive(g_time_sync_q, &msg, pdMS_TO_TICKS(1)))
        {
#if NTP_TIMESYNC
            if (msg == true)
            {
                ESP_LOGI(LOG_TAG_TASK_TIME, "Getting System Time...");
                configTime(s_gmtOffset_sec, s_daylightOffset_sec, s_ntpServer);
                if (!getLocalTime(&now))
                {
                    ESP_LOGE(LOG_TAG_TASK_TIME, "Failed to obtain time");
                    task_time_msg(true);
                }
                else
                {
                    ESP_LOGI(LOG_TAG_TASK_TIME, "Time in India: %d:%d:%d",
                             now.tm_hour,
                             now.tm_min,
                             now.tm_sec);
                    c_alarm(ALARM_TIMER_ID_0, now, tim_0_offset, true, &tim_0_alarm_cb);
                    time_synced = true;
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
                    ESP_LOGD(LOG_TAG_TASK_TIME, "LIGHTS ON");
                    sunlight = false;
                }
                else
                {
                    /* Lights Off */
                    ESP_LOGD(LOG_TAG_TASK_TIME, "LIGHTS OFF");
                    sunlight = true;
                }
                task_control_msg(CONTROL_Q_MSG_ID_SET_SUNLIGHT_DETECTED, &sunlight);
            }
#endif /* NTP_TIMESYNC */
        }
        else if(time_synced)
        {
            time(&t_now);
            if (alarm_reg[tim_itr_cntr] != NULL)
            {
                /* If the time expired */
                if (alarm_reg[tim_itr_cntr]->armed && (unsigned long)t_now >= alarm_reg[tim_itr_cntr]->alarm_time)
                {
                    ESP_LOGV(LOG_TAG_TASK_TIME, "%ld : %ld", alarm_reg[tim_itr_cntr]->alarm_time, (unsigned long)t_now);
                    /* If cb is not NULL */
                    if (alarm_reg[tim_itr_cntr]->m_alarm_cb != NULL)
                    {
                        /* Call the callback */
                        ESP_LOGV(LOG_TAG_TASK_TIME, "callback registered");
                        localtime_r(&t_now, &now);
                        
                        alarm_reg[tim_itr_cntr]->m_alarm_cb(&now);
                        if (alarm_reg[tim_itr_cntr]->reload)
                        {
                            c_alarm((alarm_timer_id)tim_itr_cntr,
                                    now,
                                    alarm_reg[tim_itr_cntr]->alarm_offset,
                                    alarm_reg[tim_itr_cntr]->reload,
                                    alarm_reg[tim_itr_cntr]->m_alarm_cb);
                        }
                        else
                        {
                            ESP_LOGV(LOG_TAG_TASK_TIME, "Deleting Timer: %d", tim_itr_cntr);
                            alarm_reg[tim_itr_cntr]->armed = false;
                        }
                    }
                }
            }

            tim_itr_cntr++;
            tim_itr_cntr %= ALARM_TIMER_ID_MAX;
        }
        vTaskDelay(pdMS_TO_TICKS(1000/ALARM_TIMER_ID_MAX));
    }
}

BaseType_t task_time_msg(bool msg_id)
{
    static bool msg;
    BaseType_t status = pdFALSE;

    msg = msg_id;

    status = xQueueSend(g_time_sync_q, &msg, portMAX_DELAY);
    if (status == pdFALSE)
    {
        ESP_LOGE(LOG_TAG_TASK_TIME, "Queue Send Failure");
    }
    return status;
}