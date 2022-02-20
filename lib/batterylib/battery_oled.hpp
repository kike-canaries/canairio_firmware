#ifndef battery_oled_hpp
#define battery_oled_hpp

#include <battery.hpp>
#include <esp_adc_cal.h>

#define BATTERY_MIN_V 3.4
#define BATTERY_MAX_V 4.04
#define BATTCHARG_MIN_V 3.69
#define BATTCHARG_MAX_V 4.198
#define ADC_EN 14

#ifdef M5PICOD4 
  #define ADC_PIN 36
else 
  #define ADC_PIN 34
#endif

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
    float curv = 0;
    void setupBattADC();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OLEDBATTERY)
extern Battery_OLED battery;
#endif

#endif

