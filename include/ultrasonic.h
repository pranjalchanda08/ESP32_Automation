#include "input.h"

/**
 * @class ultrasonic
 * @brief Represents an ultrasonic sensor.
 * @details The ultrasonic sensor is an input sensor that measures distance
 * by sending out a high frequency sound wave and then timing how long it takes
 * for the echo of that sound wave to return to the sensor.
 */
class ultrasonic : public input
{
private:
    int m_trig_pin; /**< The pin used to trigger the ultrasonic sensor. */
    int m_echo_pin; /**< The pin used to receive the echo from the ultrasonic sensor. */
    float m_distance_cm; /**< The distance measured by the ultrasonic sensor in centimeters. */
    float m_calib_cm; /**< The calibration distance for the ultrasonic sensor in centimeters. */
    SemaphoreHandle_t m_lock; /**< A semaphore used to synchronize access to the ultrasonic sensor. */
public:
    /**
     * @brief Checks if a person is detected by the ultrasonic sensor.
     * @return True if a person is detected, false otherwise.
     */
    bool isPerson();
    
    /**
     * @brief Calibrates the ultrasonic sensor.
     * @details The calibration process determines the distance between the
     * ultrasonic sensor and the object it is measuring distance to.
     */
    void caliberate();
    
    /**
     * @brief Calibrates the ultrasonic sensor using FreeRTOS task.
     * @details The calibration process determines the distance between the
     * ultrasonic sensor and the object it is measuring distance to.
     */
    void RTOS_caliberate();
    
    /**
     * @brief Gets the calibration distance for the ultrasonic sensor in centimeters.
     * @return The calibration distance for the ultrasonic sensor in centimeters.
     */
    float get_calib_cm();
    
    /**
     * @brief Gets the distance measured by the ultrasonic sensor in centimeters.
     * @return The distance measured by the ultrasonic sensor in centimeters.
     */
    float get_distance_cm();
    
    /**
     * @brief Reads the distance from the ultrasonic sensor.
     */
    void read();
    
    /**
     * @brief Constructor for the ultrasonic class.
     * @param trigger The pin used to trigger the ultrasonic sensor.
     * @param echo The pin used to receive the echo from the ultrasonic sensor.
     */
    ultrasonic(int trigger, int echo);
    
    /**
     * @brief Destructor for the ultrasonic class.
     */
    ~ultrasonic();
};
