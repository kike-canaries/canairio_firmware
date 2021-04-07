#include <WiFi.h>
#include <OTAHandler.h>
#include <esp_wifi.h>
#include <CanAirIoApi.hpp>
#include <ConfigApp.hpp>
#include <InfluxArduino.hpp>
#include <GUILib.hpp>
#include <Watchdog.hpp>
#include <Sensors.hpp>

#define PUBLISH_INTERVAL 30       // publish to cloud each 30 seconds
#define WIFI_RETRY_CONNECTION 30  // 30 seconds wait for wifi connection
#define IFX_RETRY_CONNECTION 5    // influxdb publish retry 

void otaLoop();
void otaInit();
bool wifiCheck();
void wifiConnect(const char* ssid, const char* pass);
void wifiInit();
void wifiStop();
void wifiRestart();
void wifiLoop();
int  getWifiRSSI();

bool apiIsConfigured();
void apiInit();
void apiLoop();

bool influxDbIsConfigured();
void influxDbInit();
void influxDbParseFields(char* fields);
void influxDbAddTags(char* tags);
bool influxDbWrite();
void influxDbLoop();
