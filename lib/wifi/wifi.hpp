#ifndef wifi_hpp
#define wifi_hpp

#include <WiFi.h>
#include <esp_wifi.h>
#ifndef DISABLE_CLI
#include <ESP32WifiCLI.hpp>
#endif
#include "GUILib.hpp"

#define PUBLISH_INTERVAL 30       // publish to cloud each 30 seconds
#define WIFI_RETRY_CONNECTION 30  // 30 seconds wait for wifi connection

// change these params via CLI:
#define GMT_OFFSET_SEC               (3600 * 8)
#define NTP_SERVER1                  "pool.ntp.org"
#define NTP_SERVER2                  "time.nist.gov"
#define DEFAULT_TZONE                "CET-1CEST,M3.5.0,M10.5.0/3"

void otaLoop();
void wifiInit();
void wifiStop();
void wifiRestart();
void wifiLoop();

void printLocalTime(bool onlyTime = false);
void updateTimeSettings(bool silent = false);

int  getWifiRSSI();
void printWifiRSSI();
String getDeviceInfo(bool iSCLI = true);
String getHostId();
#endif