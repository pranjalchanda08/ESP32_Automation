#ifndef _RGB_LED_H_
#define _RGB_LED_H_

#include "output.h"

/**
 * @brief A class for controlling an RGB LED with three output pins.
 */
class rgb_led
{
private:
    output m_r_pin; ///< The output pin connected to the red LED component.
    output m_g_pin; ///< The output pin connected to the green LED component.
    output m_b_pin; ///< The output pin connected to the blue LED component.

public:
    /**
     * @brief Construct a new rgb_led object with the specified output pin numbers.
     *
     * @param r The pin number for the red LED component.
     * @param g The pin number for the green LED component.
     * @param b The pin number for the blue LED component.
     */
    rgb_led(int r, int g, int b);

    /**
     * @brief Destroy the rgb_led object and free any allocated resources.
     */
    ~rgb_led();

    /**
     * @brief Set the color of the LED using a 32-bit RGB color value.
     *
     * @param rgb A 32-bit RGB color value in the format 0xRRGGBB.
     */
    void operator<<(uint32_t rgb);

    /**
     * @brief Set the color of the LED using separate red, green, and blue color values.
     *
     * @param rgb A tuple of three uint8_t values representing the red, green, and blue color components.
     */
    void operator<<(const std::tuple<uint8_t, uint8_t, uint8_t> &rgb);

    /**
     * @brief Set the color of the LED using a hex string representation of the RGB color.
     *
     * @param rgb A string representing the RGB color in hex format, e.g. "FF0000" for red.
     */
    void operator<<(String rgb);
};

#endif /*_RGB_LED_H_*/
