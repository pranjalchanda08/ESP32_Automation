#ifndef _INPUT_H_
#define _INPUT_H_

#include <Arduino.h>

class input
{
private:
    int m_pin_no; ///< The pin number that this input is connected to.
    bool m_cur_state; ///< The current state of this input.
    SemaphoreHandle_t m_lock; ///< A mutex to protect concurrent access to this input.

public:
    /**
     * @brief Reads a pulse on this input and returns the duration of the pulse.
     * 
     * This function blocks until a pulse is detected, and then returns the duration
     * of the pulse in microseconds.
     * 
     * @param state The desired state of the input that triggers the pulse (HIGH or LOW).
     * @return The duration of the pulse in microseconds.
     */
    int pulse_read(bool state);

    /**
     * @brief Reads the current state of this input and updates the internal state.
     * 
     * This function reads the current state of the input pin and updates the internal state
     * of this object. If the state has changed since the last read, the function acquires
     * the internal lock to protect concurrent access.
     */
    void read();

    /**
     * @brief Gets the current state of this input.
     * 
     * @return The current state of this input.
     */
    bool get_cur_state();

    /**
     * @brief Construct a new input object with the specified pin number.
     * 
     * @param pin_no The pin number that this input is connected to.
     */
    input(int pin_no);

    /**
     * @brief Destroy the input object and free any allocated resources.
     */
    ~input();
};

#endif /* _INPUT_H_ */
