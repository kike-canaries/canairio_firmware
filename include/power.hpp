#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_wifi.h>

#include <Batterylib.hpp>

#include "driver/rtc_io.h"

#define DEEP_SLEEP_TIME 120  // seconds

void powerDeepSleepButton();
void powerDeepSleepTimer(int);
void powerLightSleepTimer(int);
void powerInit();
