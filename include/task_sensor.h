#ifndef _TASK_SENSOR_H_
#define _TASK_SENSOR_H_

#include "main.h"

/**
 * @brief Entry point for the sensor task.
 *
 * This function is the entry point for the sensor task.
 * It reads sensor data and sends it to the control task
 * via a queue.
 *
 * @param args A pointer to task arguments. Unused in this
 *             implementation.
 */
void task_sensor (void * args);

#endif /* _TASK_SENSOR_H_ */
