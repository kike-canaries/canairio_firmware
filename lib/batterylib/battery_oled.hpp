#ifndef battery_oled_hpp
#define battery_oled_hpp

#include <battery.hpp>
#include <esp_adc_cal.h>

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

