#include "task_control.h"
#include "task_actuator.h"

#define CONTROL_QUEUE_LEN       10
#define CONTROL_QUEUE_SIZE      sizeof(control_q_msg_t)

#define TIMER_CLK_MHZ                   80
#define PERSON_ACTIVE_TIMER_NO          0
#define PERSON_ACTIVE_TIMER_PRESCALE    TIMER_CLK_MHZ * 100
#define PERSON_ACTIVE_TIMER_MATCH_VAL   PERSON_ACTIVE_TIMER_MINS * 60000

#define PERSON_ACTIVE_TIMER_MINS        2

static hw_timer_t *person_active_timer = NULL;

static void _person_detect_timer_init();
static void _person_detect_timer_trigg();
static void IRAM_ATTR _on_person_active_timer();

QueueHandle_t g_control_q;
control_t g_control_settings;

void task_control (void * args)
{
    control_q_msg_t msg;
    bool state = false;
    byte ch_switch;
    _person_detect_timer_init();
    g_control_q = xQueueCreate(CONTROL_QUEUE_LEN, CONTROL_QUEUE_SIZE);
    ESP_LOGI("tas_control", "task_control created");

    while (true)
    {
        xQueueReceive(g_control_q, (void*)&msg, portMAX_DELAY);
        ESP_LOGD("task_control", "CONTROL_Q_MSG, ID: %d", msg.control_q_msg_id);
        switch (msg.control_q_msg_id)
        {
            case CONTROL_Q_MSG_ID_SET_MASTER_SWITCH:
                state = (bool) (((int)msg.p_payload) & 0xFF);
                g_control_settings.master_switch = state;
                ESP_LOGD("task_control", "master_sw: %d", state);
                break;
            case CONTROL_Q_MSG_ID_SET_DRIVE_MODE:
                state = (bool) (((int)msg.p_payload) & 0xFF);
                g_control_settings.drive_mode = state;
                ESP_LOGD("task_control", "drive_mode: %d", state);
                break;
            case CONTROL_Q_MSG_ID_SET_PERSON_DETECTED:
                state = (bool) (((int)msg.p_payload) & 0xFF);
                g_control_settings.person_detected = state;
                ESP_LOGD("task_control", "person_detected: %d", state);
                if (g_control_settings.person_detected)
                {
                    _person_detect_timer_trigg();
                }
                break;
            case CONTROL_Q_MSG_ID_SET_PERSON_DETE_TIM_EXP:
                /* Turn off everything */
                ESP_LOGD("task_control", "Turning Off Everything");
                g_control_settings.channel_sw_aut = 0x00;
                break;
            case CONTROL_Q_MSG_ID_SET_CH_SWITCH:
                ch_switch = (byte) ((int)msg.p_payload & 0xFF);
                g_control_settings.channel_sw_man = ch_switch;
                ESP_LOGD("task_control", "Man_sw: %d", ch_switch);

                break;
            case CONTROL_Q_MSG_ID_SET_SUNLIGHT_DETECTED:
                state = (bool) (((int)msg.p_payload) & 0xFF);
                g_control_settings.sunlight = state;
                ESP_LOGD("task_control", "sunlight: %d", state);
                g_control_settings.channel_sw_aut = 
                    g_control_settings.sunlight == false ? 0xFF | (1 << RELAY_CHANNEL_LIGHT) :
                    0xFF & ~(1 << RELAY_CHANNEL_LIGHT);
                break;
            default:
                break;
        }
    }
}

BaseType_t task_control_msg (control_q_msg_id_t msg_id, void *p_payload)
{
    static control_q_msg_t msg;

    msg.control_q_msg_id = msg_id;
    msg.p_payload = p_payload;

    return xQueueSend(g_control_q, &msg, portMAX_DELAY);
}

BaseType_t task_control_msg_isr (control_q_msg_id_t msg_id, void *p_payload)
{
    static control_q_msg_t msg;

    BaseType_t yeild = pdFALSE;
    msg.control_q_msg_id = msg_id;
    msg.p_payload = p_payload;

    return xQueueSendFromISR(g_control_q, &msg, &yeild);
}

static void _person_detect_timer_trigg()
{
    timerRestart(person_active_timer);
    timerStart(person_active_timer);
    timerAlarmEnable(person_active_timer);
}

static void _person_detect_timer_init()
{
#ifndef USE_HRES_TIMER
    setCpuFrequencyMhz(TIMER_CLK_MHZ);
    ESP_LOGD("task_control", "CPU Frequency set to %ld\r\n", getCpuFrequencyMhz());

    person_active_timer = timerBegin(PERSON_ACTIVE_TIMER_NO, 
                                    PERSON_ACTIVE_TIMER_PRESCALE, 
                                    true);
    timerAttachInterrupt(person_active_timer, 
                        &_on_person_active_timer, 
                        true);
    timerAlarmWrite(person_active_timer,
                    PERSON_ACTIVE_TIMER_MATCH_VAL,
                    false); /* Single Shot */
#endif /* USE_HRES_TIMER */
}

static void IRAM_ATTR _on_person_active_timer(){
    ESP_LOGD("task_control", "Person active timer expired");
    /* Send Msg of Timer Exp */
    task_control_msg_isr(CONTROL_Q_MSG_ID_SET_PERSON_DETE_TIM_EXP, NULL);
}