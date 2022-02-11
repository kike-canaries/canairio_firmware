#include <battery_oled.hpp>
#include <esp_adc_cal.h>

#ifndef M5STICKCPLUS
  #ifndef TTGO_TDISPLAY

void Battery_OLED::init(bool debug) {
}

float Battery_OLED::getVoltage() {
      //return 0;
    return curv;
}

bool Battery_OLED::isCharging() {
    //return false;
     return curv > BATTERY_MAX_V + (BATTCHARG_MIN_V - BATTERY_MAX_V ) / 2;
}

void Battery_OLED::printValues() { 
  if (!debug) return;
    Serial.printf("-->[BATT] Battery voltage  \t: %.3fv vref: %i Charge:%i\n", curv, vref, getCharge());  //Output voltage and current of Bat
}

void Battery_OLED::update() { 
  digitalWrite(ADC_EN, HIGH);
    delay(10);                         // suggested by @ygator user in issue #2
    uint16_t v = analogRead(ADC_PIN);
    curv = ((float)v / 4095.0) * 16;
    digitalWrite(ADC_EN, LOW);   // for possible issue: https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/6
    //return curv;   
}

int Battery_OLED::getCharge() {
   if (isCharging()) {
        return calcPercentage(curv, BATTCHARG_MAX_V, BATTCHARG_MIN_V);
    } else {
        return calcPercentage(curv, BATTERY_MAX_V, BATTERY_MIN_V);
    }
    //return 0;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OLEDBATTERY)
Battery_OLED battery;
#endif

  #endif
#endif



