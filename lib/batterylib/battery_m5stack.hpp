#ifndef battery_m5stack_hpp
#define battery_m5stack_hpp

#include <battery.hpp>
#ifdef M5STICKCPLUS 
#include <M5StickCPlus.h>
#endif

class Battery_M5STACK : public Battery {
  public:
    void init(bool debug = false);
    float getVoltage();
    float getCurrent();
    bool isCharging();
    void printValues();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_M5STACKBATTERY)
extern Battery_M5STACK battery;
#endif

#endif
