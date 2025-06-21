#ifdef CONFIG_IDF_TARGET_ESP32S3 
#include "driver/temp_sensor.h"
#endif
#include <driver/rtc_io.h>
#ifndef DISABLE_BLE
#include <esp_bt.h>
#include <esp_bt_main.h>
#endif
#include <Batterylib.hpp>
#include <esp_wifi.h>



void powerCompleteShutdown();
void powerDeepSleepButton();
void powerDeepSleepTimer(int);
void powerLightSleepTimer(int);
void powerEnableSensors();
void powerDisableSensors();

float powerESP32TempRead();
int powerGetMainHwEnbPin();

void powerLoop();
void powerInit();
