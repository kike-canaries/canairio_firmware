#include <WiFi.h>
#include <esp_wifi.h>
#include <ESP32WifiCLI.hpp>
#include "GUILib.hpp"

//#define IFX_RETRY_CONNECTION 5    // influxdb publish retry 

#define PUBLISH_INTERVAL 30       // publish to cloud each 30 seconds
#define WIFI_RETRY_CONNECTION 30  // 30 seconds wait for wifi connection

void otaLoop();
void wifiInit();
void wifiStop();
void wifiRestart();
void wifiLoop();

int  getWifiRSSI();
void printWifiRSSI();
String getDeviceInfo();
String getHostId();
