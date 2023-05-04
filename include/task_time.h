#ifndef _TASK_TIME_H_
#define _TASK_TIME_H_

#include "main.h"

typedef enum
{
    ALARM_TIMER_ID_0,
    ALARM_TIMER_ID_1,
    ALARM_TIMER_ID_2,
    ALARM_TIMER_ID_3,
    ALARM_TIMER_ID_MAX,
} alarm_timer_id;

typedef void (*alarm_cb)(void *args);

class c_alarm
{
public:
    struct tm alarm_offset;
    alarm_timer_id tim_id;
    long alarm_time;
    bool reload;
    alarm_cb m_alarm_cb;

public:
    c_alarm(alarm_timer_id tim_id, struct tm alarm_offset, bool reload, alarm_cb alarm_cb);
    c_alarm(alarm_timer_id tim_id, struct tm start_time, struct tm alarm_offset, bool reload, alarm_cb alarm_cb);
    ~c_alarm();
};

void task_time(void *args);
BaseType_t task_time_msg(bool msg_id);

#endif /* _TASK_TIME_H_ */