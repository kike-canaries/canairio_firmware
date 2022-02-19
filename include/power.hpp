#include <driver/rtc_io.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_wifi.h>

#include <Batterylib.hpp>

void powerDeepSleepButton();
void powerDeepSleepTimer(int);
void powerLightSleepTimer(int);
void powerInit();
