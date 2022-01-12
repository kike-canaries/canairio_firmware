#ifndef battery_oled_hpp
#define battery_oled_hpp

#include <battery.hpp>

class Battery_OLED : public Battery {
  public:
    void init(bool debug = false);
    float getVoltage();
    uint8 getCharge();
    void isCharging();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OLEDBATTERY)
extern Battery_OLED battery;
#endif

#endif

