#ifndef battery_tft_hpp
#define battery_tft_hpp

#include <battery.hpp>
#include <esp_adc_cal.h>

#define ADC_EN 14
#define ADC_PIN 34

class Battery_TFT : public Battery {
  public:
    void init(bool debug = false);
    float getVoltage();
    bool isCharging();
    void printValues();
    void update();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TFTHANDLER)
extern Battery_TFT battery;
#endif

#endif
