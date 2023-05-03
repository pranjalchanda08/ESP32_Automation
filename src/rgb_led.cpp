#include "rgb_led.h"

void rgb_led::operator<<(const std::tuple<uint8_t, uint8_t, uint8_t>& rgb)
{
    uint8_t r = std::get<0>(rgb);
    uint8_t g = std::get<1>(rgb);
    uint8_t b = std::get<2>(rgb);
    m_r_pin<<(r);
    m_g_pin<<(g);
    m_b_pin<<(b);
}

void rgb_led::operator<<(const uint32_t rgb)
{
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    m_r_pin<<(r);
    m_g_pin<<(g);
    m_b_pin<<(b);
}

void rgb_led::operator<<(const String rgb)
{
    uint32_t color = strtol(rgb.c_str(), NULL, 16);
    *this << color;
}

rgb_led::rgb_led(int r, int g, int b)
    : m_r_pin(r), m_g_pin(g), m_b_pin(b)
{
    
}

rgb_led::~rgb_led()
{
}