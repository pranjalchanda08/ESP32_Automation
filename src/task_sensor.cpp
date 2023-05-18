#include "task_sensor.h"
#include "task_control.h"
#include "task_wifi_manager.h"
#include "mqtt.h"

#define TAG_SENSOR "TASK_SENSOR"
#define SENSOR_TASK_DEL_MS 500

#define TIMER_CLK_MHZ 80
#define PERSON_DETECT_PRESCALE 4

#define PERSON_ACTIVE_TIMER_MINS 2

static hw_timer_t *person_active_timer = NULL;

#ifdef INCLUDE_ULTRASONIC
static void _sensor_caliberate();
#endif /* INCLUDE_ULTRASONIC*/

void task_sensor(void *args)
{
    ESP_LOGI(TAG_SENSOR, "task_sensor created");

#ifdef INCLUDE_ULTRASONIC
    _sensor_caliberate();
#endif /* INCLUDE_ULTRASONIC*/
#ifdef INCLUDE_MICRO_MOTION_DET
    uint8_t motion_detect_prescale = 0;
#endif /* INCLUDE_MICRO_MOTION_DET */
    bool state;

    while (true)
    {
        g_con_sw.read();
        ESP_LOGD(TAG_SENSOR, "SW: %d", g_con_sw.get_cur_state());
        state = g_con_sw.get_cur_state();
        if (!state && (pdTRUE != task_wlan_msg(WLAN_Q_MSG_ID_BEGIN_SCONFIG, (void *)&state)))
        {
            ESP_LOGW(TAG_SENSOR, "Sensor Q data send failed");
        }
#ifdef INCLUDE_LDR
        /* Read LDR State */
        g_ldr.read();
        ESP_LOGD(TAG_SENSOR, "LDR: %d", (int)g_ldr.get_cur_state());
        if (pdTRUE != task_control_msg(CONTROL_Q_MSG_ID_SET_SUNLIGHT_DETECTED, (void *)g_ldr.get_cur_state()))
        {
            ESP_LOGW(TAG_SENSOR, "Sensor Q data send failed");
        }
#endif  /* INCLUDE_LDR */
        /* Read Ultrasonic distance */
#ifdef INCLUDE_ULTRASONIC
        g_ultrasonic.read();
        ESP_LOGD(TAG_SENSOR, "ULTRASONIC: %.2f cm", g_ultrasonic.get_distance_cm());
        state = g_ultrasonic.isPerson();
        if (state && (pdTRUE != task_control_msg(CONTROL_Q_MSG_ID_SET_MOTION_DETECTED, (void *)&state)))
        {
            ESP_LOGW(TAG_SENSOR, "Sensor Q data send failed");
        }
#endif /* INCLUDE_ULTRASONIC*/
#ifdef INCLUDE_MICRO_MOTION_DET
        g_motion_det.read();
        state = g_motion_det.get_cur_state();
        ESP_LOGD(TAG_SENSOR, "Motion Detection: %d", state);
        if (!motion_detect_prescale && state && (pdTRUE != task_control_msg(CONTROL_Q_MSG_ID_SET_MOTION_DETECTED, (void *)&state)))
        {
            ESP_LOGW(TAG_SENSOR, "Sensor Q data send failed");
        }
        motion_detect_prescale++;
        motion_detect_prescale = motion_detect_prescale % PERSON_DETECT_PRESCALE;
#endif /* INCLUDE_MICRO_MOTION_DET */
        vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_DEL_MS));
    }
}

#ifdef INCLUDE_ULTRASONIC
static void _sensor_caliberate()
{
    ESP_LOGI(TAG_SENSOR, "Sensors Caliberating...");

    g_ultrasonic.RTOS_caliberate();
    ESP_LOGD(TAG_SENSOR, "ULTRASONIC Calib: %f cm", g_ultrasonic.get_calib_cm());

    ESP_LOGI(TAG_SENSOR, "Caliberation Done..");
}
#endif /* INCLUDE_ULTRASONIC*/