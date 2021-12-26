#include "battery.hpp"

int vref = 1100;
float curv = 0;

void setupBattADC() {
    #ifndef M5STICKCPLUS
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("-->[BATT] ADC eFuse Vref:%u mV\n", adc_chars.vref);
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("-->[BATT] ADC Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
    } else {
        Serial.printf("-->[BATT] ADC Default Vref: %u mV\n", vref);
    }
    #endif
}

void setupBattery() {
    #ifndef M5STICKCPLUS
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
    #endif
}

float battGetVoltage() {
    #ifdef M5STICKCPLUS
    if (battIsCharging()) curv =  M5.Axp.GetVBusVoltage();
    else curv = M5.Axp.GetBatVoltage();
    #else
    // setupBattery();
    digitalWrite(ADC_EN, HIGH);
    delay(10);                         // suggested by @ygator user in issue #2
    uint16_t v = analogRead(ADC_PIN);
    curv = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    digitalWrite(ADC_EN, LOW);   // for possible issue: https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/6
    #endif
    return curv;
}

uint8_t _calcPercentage(float volts, float max, float min) {
    float percentage = (volts - min) * 100 / (max - min);
    if (percentage > 100) {
        percentage = 100;
    }
    if (percentage < 0) {
        percentage = 0;
    }
    return (uint8_t)percentage;
}

uint8_t battCalcPercentage(float volts) {
    if (battIsCharging()){
      return _calcPercentage(volts,BATTCHARG_MAX_V,BATTCHARG_MIN_V);
    } else {
      return _calcPercentage(volts,BATTERY_MAX_V,BATTERY_MIN_V);
    }
}

bool battIsCharging() {
    #ifdef M5STICKCPLUS
    return M5.Axp.GetVBusCurrent() > 0;
    #else
    return curv > BATTERY_MAX_V + (BATTCHARG_MIN_V - BATTERY_MAX_V ) / 2;
    #endif
}

void battPrintValues() {
    #ifdef M5STICKCPLUS
    Serial.printf("-->[BATT] AXP Temp\t: %.1fC\n", M5.Axp.GetTempInAXP192());                                          //Get the temperature of AXP192
    Serial.printf("-->[BATT] AXP Bat Volts\t: %.3fv  \tI: %.3fma\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());  //Output voltage and current of Bat
    Serial.printf("-->[BATT] AXP USB Volts\t: %.3fv  \tI: %.3fma\n", M5.Axp.GetVBusVoltage(), M5.Axp.GetVBusCurrent());  //Output current and voltage of USB
    Serial.printf("-->[BATT] AXP 5V  Volts\t: %.3fv  \tI: %.3fma\n", M5.Axp.GetVinVoltage(), M5.Axp.GetVinCurrent());
    Serial.printf("-->[BATT] AXP Bat power\t: %.3fmw\n", M5.Axp.GetBatPower());
    #else
    Serial.printf("-->[BATT] Bat Volts\t: %.3fv", curv);  //Output voltage
    #endif
}