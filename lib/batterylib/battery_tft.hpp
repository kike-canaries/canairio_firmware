#ifndef battery_tft_hpp
#define battery_tft_hpp

#include <battery.hpp>
#include <esp_adc_cal.h>

#define ADC_EN 14
#define ADC_PIN 34

#define BATTERY_MIN_V 3.4
#define BATTERY_MAX_V 4.19
#define BATTCHARG_MIN_V 4.21
#define BATTCHARG_MAX_V 4.8

class Battery_TFT : public Battery {
  public:
    void init(bool debug = false);
    float getVoltage();
    bool isCharging();
    int getCharge();
    void printValues();
    void update();
  private:
    int vref = 1100;
    void setupBattADC();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TFTHANDLER)
extern Battery_TFT battery;
#endif

#endif
