#include "task_sensor.h"
#include "task_control.h"
#include "task_wifi_manager.h"

#define TIMER_CLK_MHZ 80
#define PERSON_ACTIVE_TIMER_NO 3
#define PERSON_ACTIVE_TIMER_PRESCALE TIMER_CLK_MHZ * 100
#define PERSON_ACTIVE_TIMER_MATCH_VAL PERSON_ACTIVE_TIMER_MINS * 60000

#define PERSON_ACTIVE_TIMER_MINS 2

static hw_timer_t *person_active_timer = NULL;

static void _sensor_caliberate();
static void _person_detect_timer_init();
static void _person_detect_timer_trigg();

void task_sensor(void *args)
{
    ESP_LOGI("task_sensor", "task_sensor created");

    _sensor_caliberate();
    bool state;

    while (true)
    {
        g_con_sw.read();
        ESP_LOGD("task_sensor", "SW: %d", g_con_sw.get_cur_state());
        state = g_con_sw.get_cur_state();
        if (!state && (pdTRUE != task_wlan_msg(WLAN_Q_MSG_ID_BEGIN_SCONFIG, (void *)&state)))
        {
            ESP_LOGE("task_control", "WLAN Queue Full");
        }
#ifdef INCLUDE_LDR
        /* Read LDR State */
        g_ldr.read();
        ESP_LOGD("task_sensor", "LDR: %d", (int)g_ldr.get_cur_state());
        if (pdTRUE != task_control_msg(CONTROL_Q_MSG_ID_SET_SUNLIGHT_DETECTED, (void *)g_ldr.get_cur_state()))
        {
            ESP_LOGE("task_control", "Control Queue Full");
        }
#endif /* INCLUDE_LDR */
        /* Read Ultrasonic distance */
        g_ultrasonic.read();
        ESP_LOGD("task_sensor", "ULTRASONIC: %.2f cm", g_ultrasonic.get_distance_cm());
        state = g_ultrasonic.isPerson();
        if (state && (pdTRUE != task_control_msg(CONTROL_Q_MSG_ID_SET_PERSON_DETECTED, (void *)&state)))
        {
            ESP_LOGE("task_control", "Control Queue Full");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void _sensor_caliberate()
{
    ESP_LOGI("task_sensor", "Sensors Caliberating...");

    g_ultrasonic.RTOS_caliberate();
    ESP_LOGD("task_sensor", "ULTRASONIC Calib: %f cm",  g_ultrasonic.get_calib_cm());

    ESP_LOGI("task_sensor", "Caliberation Done..");
}