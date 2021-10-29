#include <WiFi.h>
#include <OTAHandler.h>
#include <esp_wifi.h>
#include <InfluxDbClient.h>
#include <ConfigApp.hpp>
#include <GUILib.hpp>
#include <Watchdog.hpp>
#include <Sensors.hpp>
#include "power.h"

#define PUBLISH_INTERVAL 30       // publish to cloud each 30 seconds
#define WIFI_RETRY_CONNECTION 30  // 30 seconds wait for wifi connection
#define IFX_RETRY_CONNECTION 5    // influxdb publish retry 

void otaLoop();
void otaInit();
bool wifiCheck();
void wifiInit();
void wifiStop();
void wifiRestart();
void wifiLoop();
int  getWifiRSSI();
String getDeviceInfo();

void influxDbInit();
void influxDbLoop();

