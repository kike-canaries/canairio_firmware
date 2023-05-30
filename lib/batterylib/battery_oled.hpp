#ifndef battery_oled_hpp
#define battery_oled_hpp

#include <battery.hpp>
#include <esp_adc_cal.h>

#ifdef TTGO_T7
#define BATTERY_MIN_V 3.4
#define BATTERY_MAX_V 4.28
#define BATTCHARG_MIN_V 3.8
#define BATTCHARG_MAX_V 4.34
#else
#define BATTERY_MIN_V 3.4
#define BATTERY_MAX_V 4.04
#define BATTCHARG_MIN_V 4.06
#define BATTCHARG_MAX_V 4.198
#endif

#define ADC_EN 14

class Battery_OLED : public Battery {
  public:
    void init(bool debug = false);
    float getVoltage();
    bool isCharging();
    int getCharge();
    void printValues();
    void update();
  private:
    int vref = 1086;
    void setupBattADC();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OLEDBATTERY)
extern Battery_OLED battery;
#endif

#endif

