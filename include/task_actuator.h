#ifndef _TASK_RELAY_H_
#define _TASK_RELAY_H_

#include "ser_output.h"
#include "task_control.h"

// Enumerate the available relay channels
typedef enum relay_channel
{
    RELAY_CHANNEL_FOUNTAIN      = 0, // Relay channel for the fountain
    RELAY_CHANNEL_LIGHT         = 1  // Relay channel for the lights
} relay_channel_t;

// Task function for controlling the relays
void task_actuators(void *args);

#endif /* _TASK_RELAY_H_ */
