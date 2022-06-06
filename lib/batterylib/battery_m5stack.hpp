#ifndef battery_m5stack_hpp
#define battery_m5stack_hpp

#include <battery.hpp>
#ifdef M5STICKCPLUS 
#include <M5StickCPlus.h>
#endif

#define BATTERY_MIN_V 3.4
#define BATTERY_MAX_V 4.04
#define BATTCHARG_MIN_V 4.06
#define BATTCHARG_MAX_V 4.198

class Battery_M5STACK : public Battery {
  public:
    float vusb = 0.0;
    void init(bool debug = false);
    float getVoltage();
    float getCurrent();
    int getCharge();
    bool isCharging();
    void printValues();
    void update();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_M5STACKBATTERY)
extern Battery_M5STACK battery;
#endif

#endif
