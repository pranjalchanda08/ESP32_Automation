#ifndef _SER_OUTPUT_H_
#define _SER_OUTPUT_H_

#include <Arduino.h>

/**
 * @brief A class representing a shift register output.
 *
 * This class allows data to be written to a shift register output using the Serial Peripheral Interface (SPI) protocol.
 *
 * @note To use this class, the SPI library must be included in the project.
 */
class ser_output
{
private:
    int m_clk_pin;      ///< The pin number for the shift register clock.
    int m_latch_pin;    ///< The pin number for the shift register latch.
    int m_ser_data_pin; ///< The pin number for the shift register serial data input.

public:
    byte m_data; ///< The current data stored in the shift register.

public:
    /**
     * @brief Overloaded bit-shift operator for writing data to the shift register.
     *
     * This operator allows a byte of data to be written to the shift register using the << operator.
     *
     * @param ser_data The byte of data to be written to the shift register.
     */
    void operator<<(byte ser_data);

    /**
     * @brief Writes data to the shift register.
     *
     * This function writes a byte of data to the shift register using the SPI protocol.
     *
     * @param ser_data The byte of data to be written to the shift register.
     */
    void write(byte ser_data);

    /**
     * @brief Sets the value of a single bit in the shift register data.
     *
     * This function sets the value of a single bit in the shift register data at the specified position.
     *
     * @param bit The position of the bit to be set.
     * @param state The value to set the bit to (0 or 1).
     */
    void set_bit(int bit, bool state);

    /**
     * @brief Constructor for the ser_output class.
     *
     * This constructor initializes the shift register output object with the specified pin numbers.
     *
     * @param clk The pin number for the shift register clock.
     * @param latch The pin number for the shift register latch.
     * @param ser_data The pin number for the shift register serial data input.
     */
    ser_output(int clk, int latch, int ser_data);

    /**
     * @brief Destructor for the ser_output class.
     *
     * This destructor cleans up any resources used by the shift register output object.
     */
    ~ser_output();
};

#endif /* _SER_OUTPUT_H_ */
