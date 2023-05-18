#include "main.h"
#include "task_actuator.h"

#define ACTUATOR_TASK_DEL_MS 1

void task_actuators(void *args)
{
    ESP_LOGI("task_actuator", "task_actuators created");
    while (true)
    {
        if(g_control_settings.master_switch == true)
        {
            /* If Drive mode is auto */
            if(g_control_settings.drive_mode == 0)
            {
                g_relays << (g_control_settings.channel_sw_aut & g_control_settings.channel_sw_man) * g_control_settings.person_detected;
            }
            else
            {
                /* Write the state of the channel switch irrespective of Senso val */
                g_relays << g_control_settings.channel_sw_man;
            }
        }
        else
        {
            /* Set all channel to reset */
            g_relays << 0x00;
        }
        vTaskDelay(pdMS_TO_TICKS(ACTUATOR_TASK_DEL_MS));
    }    
}