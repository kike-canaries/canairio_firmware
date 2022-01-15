#ifndef battery_oled_hpp
#define battery_oled_hpp

#include <battery.hpp>

#define BATTERY_MIN_V 3.2
#define BATTERY_MAX_V 4.1
#define BATTCHARG_MIN_V 4.65
#define BATTCHARG_MAX_V 4.8

class Battery_OLED : public Battery {
  public:
    void init(bool debug = false);
    float getVoltage();
    bool isCharging();
    int getCharge();
    void printValues();
    void update();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OLEDBATTERY)
extern Battery_OLED battery;
#endif

#endif

