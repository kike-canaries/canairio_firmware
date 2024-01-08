#include <WiFi.h>
#include <OTAHandler.h>
#include <esp_wifi.h>
#include <InfluxDbClient.h>
#include <MQTT.h>
#include <ConfigApp.hpp>
#include <GUILib.hpp>
#include <Watchdog.hpp>
#include <Sensors.hpp>
#include <power.hpp>
#include <cloud_anaire.hpp>
#include <cloud_hass.hpp>
#include <cloud_influxdb.hpp>

#ifndef DISABLE_CLI
#include <cli.hpp>
#endif

//#define IFX_RETRY_CONNECTION 5    // influxdb publish retry 

#define PUBLISH_INTERVAL 30       // publish to cloud each 30 seconds
#define WIFI_RETRY_CONNECTION 30  // 30 seconds wait for wifi connection
#define MQTT_RETRY_CONNECTION 1   // mqtt publish retry
#define MQTT_DELAYED_TIME 30      // mqtt retry connection delayed time
#define MQTT_BUFFER_SIZE 512      // mqtt buffer size

void otaLoop();
void wifiInit();
void wifiStop();
void wifiRestart();
void wifiLoop();

int  getWifiRSSI();
void printWifiRSSI();
String getDeviceInfo();
String getHostId();
