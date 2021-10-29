#ifndef _POWER_H
#define _POWER_H

#include <Arduino.h>
#include <esp_bt_main.h>
#include <esp_bt.h>
#include <esp_wifi.h>
#include "hal.hpp"

void powerDeepSleepButton();
void powerDeepSleepTimer(int);
void powerLightSleepTimer(int);

#endif
