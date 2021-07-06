#include <Arduino.h>
#include <esp_adc_cal.h>
#include "hal.hpp"

#define BATTERY_MIN_V 3.2
#define BATTERY_MAX_V 4.1
#define BATTCHARG_MIN_V 4.65
#define BATTCHARG_MAX_V 4.88

void setupBattADC();
void setupBattery();
float battGetVoltage();
uint8_t battCalcPercentage(float volts);
void battUpdateChargeStatus();
bool battIsCharging();
void adcPowerOff();

uint8_t _calcPercentage(float volts, float max, float min);