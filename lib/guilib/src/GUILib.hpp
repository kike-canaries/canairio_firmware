#ifndef GUILib_hpp
#define GUILib_hpp

#ifdef TTGO_TDISPLAY
#include <TFTUtils.hpp>
#endif

#ifdef M5STICKCPLUS
#include <TFTUtils.hpp>
#endif

#ifdef ESP32S3
#include <TFTUtils.hpp>
#endif

#ifndef M5STICKCPLUS
  #ifndef TTGO_TDISPLAY
    #ifndef ESP32S3
    #include <GUIUtils.hpp>
    #endif
  #endif
#endif

#endif
