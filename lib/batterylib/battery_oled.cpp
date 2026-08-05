#include <battery_oled.hpp>
#ifndef DISABLE_BATT

#ifdef M5PICOD4
  #define ADC_PIN 36
  int channel_atten = ADC1_CHANNEL_6;
#elif TTGO_T7
  #define ADC_PIN 35
  int channel_atten = ADC1_CHANNEL_7;
#elif TTGO_T7S3
#define ADC_PIN 2
int channel_atten = ADC_ATTEN_DB_11;  // ESP32-S3: 11dB → 0–3.1V range
#else
  #define ADC_PIN 34
  int channel_atten = 0;
#endif

float pavg = 0.0;

void Battery_OLED::setupBattADC() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      (adc_unit_t)ADC_UNIT_1, (adc_atten_t)channel_atten,
      (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
  analogReadResolution(12);
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    log_i("[BATT] ADC eFuse Vref  \t: %u mV\r\n", adc_chars.vref);
    vref = adc_chars.vref;
  } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
    log_i("[BATT] ADC Two Point coeff_a\t: %umV coeff_b:%umV\r\n",
          adc_chars.coeff_a, adc_chars.coeff_b);
  } else {
    log_i("[BATT] ADC Default Vref  \t: %u mV\r\n", vref);
  }
}

void Battery_OLED::init(bool debug) {
  this->debug = debug;
  delay(10);
  setupBattADC();
  delay(10);
}

float Battery_OLED::getVoltage() { return curv; }

bool Battery_OLED::isCharging() {
  bool charging = false;
  if (isDischarging >= 0)
    charging = !isDischarging;
  else
    charging = curv > btDiscVMax + (btCharVMin - btDiscVMax) / 2;
  return charging;
}

void Battery_OLED::printValues() {
  if (!debug)
    return;
  Serial.printf("-->[BATT] Battery voltage  \t: %.3fv vref: %i Charge:%i\r\n",
                curv, vref, getCharge());
}

void Battery_OLED::update() {
  delay(10);
  uint16_t v = analogRead(ADC_PIN);
#if defined(TTGO_T7)
  curv = ((float)v / 4095.0) * 7.58;   // ESP32: 11dB → ~3.9V, divider 1:2
#elif defined(TTGO_T7S3)
  curv = ((float)v / 4095.0) * 6.2;    // ESP32-S3: 11dB → 3.1V, divider 1:2
#else
  curv = ((float)v / 4095.0) * 15.83;
#endif
}

int Battery_OLED::getCharge() {
  if (isCharging()) {
    return calcPercentage(curv, btCharVMax, btCharVMin);
  } else {
    return calcPercentage(curv, btDiscVMax, btDiscVMin);
  }
}
#endif
#ifdef DISABLE_BATT
void init(bool debug = false) {}
float getVoltage() { return 0.0; }
bool isCharging() { return false; }
int getCharge() { return 0; }
void printValues() {}
void update() {}
#endif

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OLEDBATTERY)
#ifndef M5STICKCPLUS
#ifndef TTGO_TDISPLAY
Battery_OLED battery;
#endif
#endif
#endif
