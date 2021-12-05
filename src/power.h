#ifndef _POWER_H
#define _POWER_H

#include <Arduino.h>
#include <esp_bt_main.h>
#include <esp_bt.h>
#include <esp_wifi.h>
#include "hal.hpp"
#include "driver/rtc_io.h"

#ifndef TTGO_TDISPLAY 
#include <GUIUtils.hpp>
#endif

void powerDeepSleepButton();
void powerDeepSleepTimer(int);
void powerLightSleepTimer(int);

#endif
