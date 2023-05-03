#ifndef _STORAHE_H_
#define _STORAHE_H_

#include <Arduino.h>
#include "Preferences.h"

class storage
{
public:
    Preferences m_pref;
public:
    bool store_SSID(String ssid);
    bool store_PSK(String ssid);
    bool store_actuator_man(uint8_t state);
    bool store_actuator_aut(uint8_t state);

    String get_stored_SSID();
    String get_stored_PSK();
    bool get_actuator_man(uint8_t * val);
    bool get_actuator_aut(uint8_t * val);

    Preferences get_mPref();

    storage();
    ~storage();
};

extern storage g_storage_struct;

#endif /* _STORAHE_H_ */