#include "ser_output.h"

ser_output::ser_output(int clk, int latch, int ser_data)
{
    this->m_clk_pin = clk;
    this->m_latch_pin = latch;
    this->m_ser_data_pin = ser_data;

    pinMode(this->m_clk_pin, OUTPUT);
    pinMode(this->m_latch_pin, OUTPUT);
    pinMode(this->m_ser_data_pin, OUTPUT);
}

void ser_output::write(byte ser_data)
{
    this->m_data = ser_data;
    digitalWrite(this->m_latch_pin, LOW);
    shiftOut(this->m_ser_data_pin, this->m_clk_pin, MSBFIRST, ser_data);
    digitalWrite(this->m_latch_pin, HIGH);
}

void ser_output::operator<<(byte ser_data)
{
    write(ser_data);
}

void ser_output::set_bit(int bit, bool state)
{
    if (state == true)
    {
        m_data |= (1 << bit);
    }
    else
    {
        m_data &= ~(1 << bit);
    }
    write(m_data);
}

ser_output::~ser_output()
{
}