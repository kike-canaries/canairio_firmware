#include <battery_m5stack.hpp>

#ifndef M5STICKCPLUS
  #ifndef TTGO_TDISPLAY

void Battery_OLED::init(bool debug) {
}

float Battery_OLED::getVoltage() {
}

bool Battery_OLED::isCharging() {
}

void Battery_OLED::printValues() {    
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OLEDBATTERY)
Battery_OLED battery;
#endif

  #endif
#endif



