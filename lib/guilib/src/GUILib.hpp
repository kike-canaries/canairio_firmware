#ifndef GUILib_hpp
#define GUILib_hpp

#ifdef TTGO_TDISPLAY
#include <TFTUtils.hpp>
#endif

#ifdef M5STICKCPLUS
#include <M5TFTUtils.hpp>
#endif

#ifndef M5STICKCPLUS
  #ifndef TTGO_TDISPLAY
  #include <GUIUtils.hpp>
  #endif
#endif

#endif
