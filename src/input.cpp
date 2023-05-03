#include "input.h"

input::input(int pin_no)
{
    this->m_pin_no = pin_no;
    pinMode(this->m_pin_no, INPUT);
    this->m_cur_state = false;

    this->m_lock = xSemaphoreCreateBinary();
    xSemaphoreGive(m_lock);
}

int input::pulse_read(bool state)
{
    return pulseIn(this->m_pin_no, state);
}

void input::read()
{
    xSemaphoreTake(m_lock, portMAX_DELAY);
    m_cur_state = digitalRead(this->m_pin_no);
    xSemaphoreGive(m_lock);
}

bool input::get_cur_state()
{
    bool curr_state;
    xSemaphoreTake(m_lock, portMAX_DELAY);
    curr_state = m_cur_state;
    xSemaphoreGive(m_lock);

    return curr_state;
}

input::~input()
{
}
