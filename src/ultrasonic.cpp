#include "ultrasonic.h"

#define SOUND_SPEED     0.034   // cm/us
#define CALIB_RETRY     10

ultrasonic::ultrasonic(int trigger, int echo)
: input(echo)
{
    this->m_trig_pin = trigger;
    this->m_echo_pin = echo;
    this->m_calib_cm = 0;
    this->m_distance_cm = 0;
    digitalWrite(this->m_trig_pin, HIGH);
    m_lock = xSemaphoreCreateBinary();
    xSemaphoreGive(m_lock);
}

float ultrasonic::get_calib_cm()
{
    return m_calib_cm;
}

float ultrasonic::get_distance_cm()
{   
    float distance_cm;
    xSemaphoreTake(m_lock, portMAX_DELAY);
    distance_cm = m_distance_cm;
    xSemaphoreGive(m_lock);

    return distance_cm;
}
void ultrasonic::read()
{
    int duration;
    xSemaphoreTake(m_lock, portMAX_DELAY);
    /* distance logic */
    digitalWrite(this->m_trig_pin, LOW);
    delayMicroseconds(2);
    digitalWrite(this->m_trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(this->m_trig_pin, LOW);

    duration = pulse_read(HIGH);
    m_distance_cm = (duration * SOUND_SPEED) / 2;
    xSemaphoreGive(m_lock);
}

bool ultrasonic::isPerson()
{
    return get_distance_cm() > get_calib_cm();
}

void ultrasonic::caliberate()
{
    uint8_t i = CALIB_RETRY;

    while (i)
    {
        read();
        m_calib_cm += get_distance_cm();
        i--;
        delay(100);
    }   

    m_calib_cm /= CALIB_RETRY;
    ESP_LOGD("ultrasonic", "Calib data: %f", m_calib_cm);
}
void ultrasonic::RTOS_caliberate()
{
    uint8_t i = CALIB_RETRY;

    while (i)
    {
        read();
        m_calib_cm += get_distance_cm();
        i--;
        ESP_LOGI("ultrasonic", "Calib retry: %d", i);
        vTaskDelay(pdMS_TO_TICKS(50));
    }   

    m_calib_cm /= CALIB_RETRY;
    ESP_LOGD("ultrasonic", "Calib data: %f", m_calib_cm);
}

ultrasonic::~ultrasonic()
{
}