#include <esp_bt_main.h>
#include <esp_bt.h>
#include <esp_wifi.h>
#include "driver/rtc_io.h"
#include <Batterylib.hpp>

#define DEEP_SLEEP_TIME 60       // seconds

void powerDeepSleepButton();
void powerDeepSleepTimer(int);
void powerLightSleepTimer(int);
void powerInit();
