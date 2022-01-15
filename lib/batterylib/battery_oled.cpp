#include <battery_oled.hpp>

#ifndef M5STICKCPLUS
  #ifndef TTGO_TDISPLAY

void Battery_OLED::init(bool debug) {
}

float Battery_OLED::getVoltage() {
    return 0;
}

bool Battery_OLED::isCharging() {
    return false;
}

void Battery_OLED::printValues() {    
}

void Battery_OLED::update() {    
}



#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OLEDBATTERY)
Battery_OLED battery;
#endif

  #endif
#endif



