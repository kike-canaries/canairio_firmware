#include "battery.hpp"

int vref = 1100;
float curv = 0;

void setupBattADC() {
    
}

void setupBattery() {
    
}

float battGetVoltage() {
    curv = M5.Axp.GetBatVoltage();
    // Serial.printf("-->[BATT] AXP Temp\t: %.1fC\n", M5.Axp.GetTempInAXP192()); //Get the temperature of AXP192. 获取AXP192的温度
    // Serial.printf("-->[BATT] AXP Bat Volts\t: %.3fv  \tI: %.3fma\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());  //Output voltage and current of Bat. 输出Bat的电压和电流
    // Serial.printf("-->[BATT] AXP USB Volts\t: %.3fv  \tI: %.3fma\n", M5.Axp.GetVBusVoltage(), M5.Axp.GetVBusCurrent());  //Output current and voltage of USB. 输出USB的电流和电压
    // Serial.printf("-->[BATT] AXP 5V  Volts\t: %.3fv  \tI: %.3fma\n", M5.Axp.GetVinVoltage(), M5.Axp.GetVinCurrent());
    // Serial.printf("-->[BATT] AXP Bat power\t: %.3fmw\n", M5.Axp.GetBatPower());
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

void battUpdateChargeStatus() {
    // digitalWrite(LED_PIN, battIsCharging());
}

bool battIsCharging() {
    return curv > BATTERY_MAX_V + (BATTCHARG_MIN_V - BATTERY_MAX_V ) / 2;
}