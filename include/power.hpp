#ifdef CONFIG_IDF_TARGET_ESP32S3 
#include "driver/temp_sensor.h"
#endif
#include <driver/rtc_io.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_wifi.h>

#include <Batterylib.hpp>

float powerESP32TempRead();
void powerCompleteShutdown();
void powerDeepSleepButton();
void powerDeepSleepTimer(int);
void powerLightSleepTimer(int);
void powerEnableSensors();
void powerDisableSensors();
void powerLoop();
void powerInit();
