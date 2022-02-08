#ifndef Batterylib_hpp
#define Batterylib_hpp

#ifdef TTGO_TDISPLAY
#include <battery_tft.hpp>
#endif

#ifdef M5STICKCPLUS
#include <battery_m5stack.hpp>
#endif

#ifndef M5STICKCPLUS
  #ifndef TTGO_TDISPLAY
  #include <battery_oled.hpp>
  #endif
#endif

#endif
