#include "task_control.h"
#include "task_actuator.h"
#include "task_time.h"
#include "mqtt.h"

#define TAG_TASK_CONTROL "TASK_CONTROL"
#define CONTROL_QUEUE_LEN 10
#define CONTROL_QUEUE_SIZE sizeof(control_q_msg_t)

#define TIMER_CLK_MHZ 80
#define PERSON_ACTIVE_TIMER_NO 0
#define PERSON_ACTIVE_TIMER_PRESCALE TIMER_CLK_MHZ * 100
#define PERSON_ACTIVE_TIMER_MATCH_VAL PERSON_ACTIVE_TIMER_MINS * 60000

#define PERSON_ACTIVE_TIMER_MINS 2

static hw_timer_t *person_active_timer = NULL;

QueueHandle_t g_control_q;
control_t g_control_settings;

void tim_1_alarm_cb(void *args)
{
    bool msg_rx;
    ESP_LOGV(TAG_TASK_CONTROL, "tim_1_alarm_cb");
    ESP_LOGI("TIM1_ALARM", "Person detection timeout. Switching off Appliances");
    task_control_msg_isr(CONTROL_Q_MSG_ID_SET_PERSON_DETE_TIM_EXP, NULL);
}

void task_control(void *args)
{
    control_q_msg_t msg;
    bool state = false;
    String pub;

    struct tm tim_1_offset;

    tim_1_offset.tm_hour = 0;
    tim_1_offset.tm_min = 2;
    tim_1_offset.tm_sec = 0;

    g_control_q = xQueueCreate(CONTROL_QUEUE_LEN, CONTROL_QUEUE_SIZE);

    ESP_LOGI(TAG_TASK_CONTROL, "task_control created");

    while (true)
    {
        xQueueReceive(g_control_q, (void *)&msg, portMAX_DELAY);
        ESP_LOGD(TAG_TASK_CONTROL, "CONTROL_Q_MSG, ID: %d", msg.control_q_msg_id);
        memcpy(&state, msg.p_payload, sizeof(bool));
        switch (msg.control_q_msg_id)
        {
        case CONTROL_Q_MSG_ID_SET_MASTER_SWITCH:
            g_control_settings.master_switch = state;
            ESP_LOGD(TAG_TASK_CONTROL, "master_sw: %d", state);
            break;
        case CONTROL_Q_MSG_ID_SET_DRIVE_MODE:
            g_control_settings.drive_mode = state;
            ESP_LOGD(TAG_TASK_CONTROL, "drive_mode: %d", state);
            break;
        case CONTROL_Q_MSG_ID_SET_MOTION_DETECTED:
            g_control_settings.person_detected = state;
            ESP_LOGD(TAG_TASK_CONTROL, "person_detected: %d", state);
            pub = state;
            mqtt_publish(PUB_TOPIC_ID_MOTION, pub.c_str(), false);
            c_alarm(ALARM_TIMER_ID_1, tim_1_offset, false, &tim_1_alarm_cb);
            break;
        case CONTROL_Q_MSG_ID_SET_PERSON_DETE_TIM_EXP:
            /* Turn off everything */
            ESP_LOGD(TAG_TASK_CONTROL, "Turning Off Everything");
            g_control_settings.person_detected = false;
            pub = false;
            mqtt_publish(PUB_TOPIC_ID_MOTION, pub.c_str(), false);
            break;
        case CONTROL_Q_MSG_ID_SET_CH0_SWITCH:
            if (state)
            {
                g_control_settings.channel_sw_man |= (1 << 0);
            }
            else
            {
                g_control_settings.channel_sw_man &= ~(1 << 0);
            }
            ESP_LOGD(TAG_TASK_CONTROL, "CH0: %d", state);

            break;
        case CONTROL_Q_MSG_ID_SET_CH1_SWITCH:
            if (state)
            {
                g_control_settings.channel_sw_man |= (1 << 1);
            }
            else
            {
                g_control_settings.channel_sw_man &= ~(1 << 1);
            }
            ESP_LOGD(TAG_TASK_CONTROL, "CH1: %d", state);

            break;
        case CONTROL_Q_MSG_ID_SET_CH2_SWITCH:
            if (state)
            {
                g_control_settings.channel_sw_man |= (1 << 2);
            }
            else
            {
                g_control_settings.channel_sw_man &= ~(1 << 2);
            }
            ESP_LOGD(TAG_TASK_CONTROL, "CH1: %d", state);

            break;
        case CONTROL_Q_MSG_ID_SET_CH3_SWITCH:
            if (state)
            {
                g_control_settings.channel_sw_man |= (1 << 3);
            }
            else
            {
                g_control_settings.channel_sw_man &= ~(1 << 3);
            }
            ESP_LOGD(TAG_TASK_CONTROL, "CH3: %d", state);

            break;
        case CONTROL_Q_MSG_ID_SET_SUNLIGHT_DETECTED:
            g_control_settings.sunlight = state;
            ESP_LOGD(TAG_TASK_CONTROL, "sunlight: %d", state);
            pub = state;
            mqtt_publish(PUB_TOPIC_ID_SUNLIGHT, pub.c_str(), false);

#ifdef INCLUDE_ULTRASONIC
            pub = ((int)g_ultrasonic.get_distance_cm());
            mqtt_publish(PUB_TOPIC_ID_ULTRASONIC, pub.c_str(), true);
#endif /* INCLUDE_ULTRASONIC*/

            g_control_settings.channel_sw_aut =
                g_control_settings.sunlight == false ? 0xFF | (1 << RELAY_CHANNEL_LIGHT) : 0xFF & ~(1 << RELAY_CHANNEL_LIGHT);
            break;
        default:
            break;
        }
    }
}

BaseType_t task_control_msg(control_q_msg_id_t msg_id, void *p_payload)
{
    static control_q_msg_t msg;

    msg.control_q_msg_id = msg_id;
    msg.p_payload = p_payload;

    return xQueueSend(g_control_q, &msg, portMAX_DELAY);
}

BaseType_t task_control_msg_isr(control_q_msg_id_t msg_id, void *p_payload)
{
    static control_q_msg_t msg;

    BaseType_t yeild = pdFALSE;
    msg.control_q_msg_id = msg_id;
    msg.p_payload = p_payload;

    return xQueueSendFromISR(g_control_q, &msg, &yeild);
}
