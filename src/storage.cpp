#include "storage.h"

storage g_storage_struct;

bool storage::store_SSID(String ssid)
{
    bool status = false;
    if (ssid.length() > 32)
    {
        status = false;
    }
    else
    {
        if (m_pref.putString("wlan_ssid", ssid) == ssid.length())
            status = true;
        else
        {
            ESP_LOGE("storage", "SSID save Failed");
        }
    }
    return status;
}
bool storage::store_PSK(String psk)
{
    bool status = false;
    if (psk.length() > 64)
    {
        status = false;
    }
    else
    {
        if (m_pref.putString("wlan_psk", psk) == psk.length())
            status = true;
        else
        {
            ESP_LOGE("storage", "PSK save Failed");
        }
    }
    return status;
}

bool storage::store_actuator_man(uint8_t state)
{
    bool status = false;

    uint16_t save = 0xAAFF & state;

    if (m_pref.putUShort("actuator_man", save))
    {
        status = true;
    }
    else
    {
        ESP_LOGE("storage", "actuator_man Failed");
    }
    return status;
}

bool storage::store_actuator_aut(uint8_t state)
{
    bool status = false;

    uint16_t save = 0xAAFF & state;

    if (m_pref.putUShort("actuator_aut", save))
    {
        status = true;
    }
    else
    {
        ESP_LOGE("storage", "actuator_aut Failed");
    }
    return status;
}

String storage::get_stored_SSID()
{
    return m_pref.getString("wlan_ssid", "DEADBEEF");
}

String storage::get_stored_PSK()
{
    return m_pref.getString("wlan_psk", "DEADBEEF");
}

bool storage::get_actuator_man(uint8_t * val)
{
    uint16_t save = 0x0000;
    save = m_pref.getUShort("actuator_man", 0xDEAD);

    if (save == 0xDEAD)
    {
        *val = 0;
        return false;
    }
    *val = 0x00FF & save;
    return true;
} 

bool storage::get_actuator_aut(uint8_t * val)
{
    uint16_t save = 0x0000;
    save = m_pref.getUShort("actuator_aut", 0xDEAD);

    if (save == 0xDEAD)
    {
        *val = 0;
        return false;
    }
    *val = 0x00FF & save;
    return true;
}

Preferences storage::get_mPref()
{
    return this->m_pref;
}

storage::storage()
{
    
}

storage::~storage()
{
    m_pref.end();
}