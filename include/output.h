#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include "Arduino.h"

class output
{
private:
    int m_pin_no; ///< The pin number that this output is connected to.
public:
    /**
     * @brief Set the state of this output to the specified boolean value.
     *
     * @param state The desired state of this output (HIGH or LOW).
     */
    void operator<<(bool state);

    /**
     * @brief Set the pulse width modulation (PWM) value of this output.
     *
     * @param pwm The desired PWM value for this output (0-255).
     */
    void operator<<(uint8_t pwm);

    /**
     * @brief Construct a new output object with the specified pin number.
     *
     * @param pin The pin number that this output is connected to.
     */
    output(int pin);

    /**
     * @brief Destroy the output object and free any allocated resources.
     */
    ~output();
};

#endif /* _OUTPUT_H_ */
