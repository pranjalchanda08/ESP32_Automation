/**
 * @file task_wifi_manager.h
 * @brief Header file for WiFi Manager task.
 */

#ifndef _TASK_WIFI_MANAGER_H_
#define _TASK_WIFI_MANAGER_H_

#include "main.h"

typedef enum wlan_q_msg_id
{
    WLAN_Q_MSG_ID_BEGIN_SCONFIG, ///< Message ID for WLAN Smart Config.
    WLAN_Q_MSG_ID_BEGIN_WLAN_CONNECT, ///< Message ID for WLAN Connect.
    WLAN_Q_MSG_ID_BEGIN_MQTT_CONNECT ///< Message ID for MQTT Start.
} wlan_q_msg_id_t;

typedef struct wlan_q_msg
{
    wlan_q_msg_id_t wlan_q_msg_id; ///< Message ID.
    void *p_payload;               ///< Pointer to the message payload.
} wlan_q_msg_t;

typedef struct wlan_struct
{
    u_long wlan_connect_retry; ///< Number of times to retry connecting to WLAN.
    bool wlan_connect_status;  ///< Current Connection Status.
    bool wlan_connect_in_progress;  ///< Current Connection Status.
} wlan_struct_t;

/**
 * @brief Task to handle WiFi management.
 *
 * This task is responsible for connecting to the WiFi network and
 * handling Smart Config (if enabled).
 *
 * @param args A pointer to the task argument (not used in this task).
 */
void task_wifi_manager(void *args);

/**
 * @brief Send a message to the WLAN task.
 *
 * This function is used to send a message to the WLAN task.
 *
 * @param msg_id The message ID.
 * @param p_payload A pointer to the message payload.
 * @return pdTRUE if the message was successfully sent, pdFALSE otherwise.
 */
BaseType_t task_wlan_msg(wlan_q_msg_id_t msg_id, void *p_payload);

/**
 * @brief Send an interrupt service routine (ISR) message to the WLAN task.
 *
 * This function is used to send an ISR message to the WLAN task.
 *
 * @param msg_id The message ID.
 * @param p_payload A pointer to the message payload.
 * @return pdTRUE if the message was successfully sent, pdFALSE otherwise.
 */
BaseType_t task_wlan_msg_isr(wlan_q_msg_id_t msg_id, void *p_payload);

#endif /* _TASK_WIFI_MANAGER_H_ */
