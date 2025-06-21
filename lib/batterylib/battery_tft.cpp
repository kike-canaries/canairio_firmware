#include <battery_tft.hpp>
#ifndef DISABLE_BATT

void Battery_TFT::setupBattADC() {
    // TODO: all here is deprecated we need review the documentation
    int channel_atten = 0;
    #ifdef ADC1_CHANNEL_6
    channel_atten = ADC1_CHANNEL_6;
    #endif
    if (strcmp(FAMILY, "ESP32-C3") == 0) return;
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)channel_atten, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        log_i("[BATT] ADC eFuse Vref  \t: %u mV\r\n", adc_chars.vref);
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        log_i("[BATT] ADC Two Point coeff_a \t: %umV coeff_b:%umV\r\n", adc_chars.coeff_a, adc_chars.coeff_b);
    } else {
        log_i("[BATT] ADC Default Vref  \t: %u mV\r\n", vref);
    }
}

void Battery_TFT::init(bool debug) {
    this->debug = debug;
    /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */
    pinMode(ADC_EN, OUTPUT);
    digitalWrite(ADC_EN, HIGH);
    delay(10);                         // suggested by @ygator user in issue #2
    setupBattADC();
    delay(10);                         // suggested by @ygator user in issue #2
    // setLimits(BATTERY_MIN_V, BATTERY_MAX_V, BATTCHARG_MIN_V, BATTCHARG_MAX_V);
}

float Battery_TFT::getVoltage () {
    return curv;
}

void Battery_TFT::update() {
    digitalWrite(ADC_EN, HIGH);
    delay(10);  // suggested by @ygator user in issue #2
    uint16_t v = analogRead(ADC_PIN);
    curv = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    if (!captureStage) digitalWrite(ADC_EN, LOW);  // for possible issue: https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/6
}

bool Battery_TFT::isCharging() {
  bool charging = false;
  if (btDiscVMax <= btCharVMin && isDischarging >= 0)
    charging = !isDischarging;
  else
    charging = curv > btDiscVMax + (btCharVMin - btDiscVMax) / 2;
//   if (debug) Serial.printf("-->[BATT] Batt is charging\t: %s\r\n", charging ? "True" : "False");
  return charging;
}

int Battery_TFT::getCharge() {
    if (isCharging()) {
        return calcPercentage(curv, this->btCharVMax, this->btCharVMin);
    } else {
        return calcPercentage(curv, this->btDiscVMax, this->btDiscVMin);
    }
}

void Battery_TFT::printValues() {
    if (!debug) return;
    Serial.printf("-->[BATT] Battery voltage  \t: %.3fv vref: %i Charge: %i\r\n", curv, vref, getCharge());  //Output voltage and current of Bat
}
#endif

#ifdef DISABLE_BATT
void init(bool debug = false) {}
float getVoltage() { return 0.0; }
bool isCharging() { return false; }
int getCharge()   { return 0; }
void printValues() {}
void update() {}
#endif

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TFTBATTERY)
Battery_TFT battery;
#endif