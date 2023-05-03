#include  "output.h"

output::output(int pin)
{
    m_pin_no = pin;
    pinMode(m_pin_no, OUTPUT);
}

void output::operator<<(bool state)
{
    digitalWrite(m_pin_no, state);
}

void output::operator<<(uint8_t pwm)
{
    analogWrite(m_pin_no, pwm);
}

output::~output()
{
}