#ifndef _TASK_CONTROL_H
#define _TASK_CONTROL_H

#include "main.h"

/**
 * @brief ID for different control queue messages.
 */
typedef enum control_q_msg_id
{
    CONTROL_Q_MSG_ID_SET_MASTER_SWITCH,        /**< Set master switch state */
    CONTROL_Q_MSG_ID_SET_DRIVE_MODE,           /**< Set drive mode (Auto/Manual) */
    CONTROL_Q_MSG_ID_SET_CH_SWITCH,            /**< Set channel switch state (Manual) */
    CONTROL_Q_MSG_ID_SET_PERSON_DETECTED,      /**< Set person detected state */
    CONTROL_Q_MSG_ID_SET_PERSON_DETE_TIM_EXP,  /**< Set person detection timer expiry state */
    CONTROL_Q_MSG_ID_SET_SUNLIGHT_DETECTED,    /**< Set sunlight detected state */
}control_q_msg_id_t;

/**
 * @brief Structure for storing control settings.
 */
typedef struct control
{
    bool master_switch;     /**< Master switch for all actuators */
    byte channel_sw_man;    /**< Per channel switch (Manual) */
    byte channel_sw_aut;    /**< Per channel switch (Automatic) */
    bool drive_mode;        /**< Drive mode (Auto/Manual) */
    bool person_detected;   /**< Person detected */
    bool sunlight;          /**< Sunlight detected */
}control_t;

/**
 * @brief Structure for control queue messages.
 */
typedef struct control_q_msg
{
    control_q_msg_id_t control_q_msg_id;    /**< Control queue message ID */
    void * p_payload;                      /**< Control queue message payload */
} control_q_msg_t;

extern QueueHandle_t g_control_q;       /**< Control queue handle */
extern control_t g_control_settings;    /**< Control settings */

/**
 * @brief Task function for the control module.
 * 
 * @param args Argument for the task (not used).
 */
void task_control (void * args);

/**
 * @brief Function to send a control message to the control task.
 * 
 * @param msg_id ID of the control message to send.
 * @param p_payload Pointer to the payload of the control message.
 * 
 * @return BaseType_t Return status of the xQueueSend() function.
 */
BaseType_t task_control_msg (control_q_msg_id_t msg_id, void *p_payload);

/**
 * @brief ISR function to send a control message to the control task.
 * 
 * @param msg_id ID of the control message to send.
 * @param p_payload Pointer to the payload of the control message.
 * 
 * @return BaseType_t Return status of the xQueueSendFromISR() function.
 */
BaseType_t task_control_msg_isr (control_q_msg_id_t msg_id, void *p_payload);

#endif /* _TASK_CONTROL_H */
