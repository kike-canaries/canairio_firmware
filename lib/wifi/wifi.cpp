#include <wifi.hpp>
#include "OTAHandler.h"
#ifndef DISABLE_CLI
#include "cli.hpp"
#endif
#include "ConfigApp.hpp"
#include "cloud_influxdb.hpp"
#include "cloud_hass.hpp"
#include "cloud_anaire.hpp"
#include "Watchdog.hpp"
#include "power.hpp"
#include <math.h>
#include <time.h>

/******************************************************************************
*   W I F I   M E T H O D S
******************************************************************************/

class MyOTAHandlerCallbacks : public OTAHandlerCallbacks {
  void onStart() {
    gui.showWelcome();
  };
  void onProgress(unsigned int progress, unsigned int total) {
    gui.showProgress(progress, total);
  };
  void onEnd() {
    gui.showWelcome();
    gui.welcomeAddMessage("");
    gui.welcomeAddMessage("success!");
    delay(2000);
    gui.welcomeAddMessage("rebooting..");
    delay(3000);
  }
  void onError() {
    gui.showWelcome();
    gui.welcomeAddMessage("");
    gui.welcomeAddMessage("!OTA Error!");
    gui.welcomeAddMessage("!Please try again!");
    delay(5000);
    gui.showWelcome();
    gui.showMain();
  }
};

namespace {
static bool isTimeValid(time_t t) {
  return t > 1609459200;  // 2021-01-01T00:00:00Z
}

static bool getTimezoneOffsetFromGeo(int32_t &offsetSeconds) {
  double lat = cfg.getDouble("lat", 0.0);
  double lon = cfg.getDouble("lon", 0.0);
  String geo = cfg.getString("geo", "");

  if ((lat == 0.0 || lon == 0.0) && geo.length() > 5) {
    Geohash gh;
    float tlat = 0.0f;
    float tlon = 0.0f;
    gh.decode(geo.c_str(), geo.length(), &tlon, &tlat);
    lat = tlat;
    lon = tlon;
  }

  if (lat == 0.0 || lon == 0.0) return false;

  int32_t offsetHours = (int32_t)lround(lon / 15.0);
  if (offsetHours < -12) offsetHours = -12;
  if (offsetHours > 14) offsetHours = 14;
  offsetSeconds = offsetHours * 3600;
  return true;
}

static void ensureNtpSync() {
  if (!WiFi.isConnected()) return;

  static bool ntpConfigured = false;
  static bool ntpSynced = false;
  static uint32_t lastAttemptMs = 0;
  static int32_t lastOffsetSeconds = INT32_MIN;

  if (ntpSynced && isTimeValid(time(nullptr))) return;

  if (millis() - lastAttemptMs < 60000) return;  // retry at most every 60s
  lastAttemptMs = millis();

  int32_t offsetSeconds = 0;
  if (!getTimezoneOffsetFromGeo(offsetSeconds)) {
    offsetSeconds = 0;
  }

  if (!ntpConfigured || offsetSeconds != lastOffsetSeconds) {
    configTime(offsetSeconds, 0, "pool.ntp.org", "time.nist.gov");
    ntpConfigured = true;
    ntpSynced = false;
    lastOffsetSeconds = offsetSeconds;
    Serial.printf("-->[WIFI] TZ offset set to\t: %ld sec\r\n", (long)offsetSeconds);
  }

  time_t now = time(nullptr);
  if (isTimeValid(now)) {
    ntpSynced = true;
    Serial.println("-->[WIFI] NTP sync ok");
  } else {
    Serial.println("-->[WIFI] NTP sync pending...");
  }
}
}  // namespace

void otaLoop() {
  if (WiFi.isConnected()) {
    wd.pause();
    ota.loop();
    wd.resume();
  }
}

void onUpdateMessage(const char* msg) {
  gui.suspendTaskGUI();
  gui.showWelcome();
  gui.welcomeAddMessage("");
  gui.welcomeAddMessage("Updating to:");
  gui.welcomeAddMessage(msg);
  gui.welcomeAddMessage("please wait..");
}

String getHostId() {
  return "CanAirIO" + getDeviceIdShort();
}

void otaInit() {
  wd.pause();
  ota.setup(getHostId().c_str(), "CanAirIO");
  gui.displayBottomLine(getHostId());
  ota.setCallbacks(new MyOTAHandlerCallbacks());
  ota.setOnUpdateMessageCb(&onUpdateMessage);
  ota.checkRemoteOTA();
  wd.resume();
}

void wifiCloudsInit() {
  influxDbInit();
  if (cfg.getBool(CONFKEYS::KANAIRE,false)) anaireInit();
  if (cfg.getBool(CONFKEYS::KHOMEAS,false)) hassInit();
  if (anaireIsConnected()) Serial.printf("-->[MQTT] %s\t: connected!\r\n", ANAIRE_HOST);
  if (hassIsConnected()) Serial.printf("-->[MQTT] Home Assistant  \t: connected!\r\n");
}

void wifiConnect() {
  String ssid = cfg.getString(CONFKEYS::KSSID, "");
  String pass = cfg.getString(CONFKEYS::KPASS, "");
  #ifndef DISABLE_CLI
  if (!(wcli.getCurrentSSID().compareTo(ssid)==0)){
    Serial.printf("-->[WIFI] saving wifi ssid\t: %s\r\n", ssid.c_str());
    saveWifi(ssid, pass);
    return;
  }
  Serial.print("-->[WIFI] connecting to wifi\t: ");
  Serial.print(ssid);
  wcli.wifiAPConnect(false);
  if (WiFi.isConnected()) {
    Serial.println(" done."); 
  }
  #else 
  if (WiFi.isConnected()) return;
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid.c_str(), pass.c_str());
  #if CONFIG_IDF_TARGET_ESP32C3
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  #endif
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry++ < WIFI_RETRY_CONNECTION) {  // M5Atom will connect automatically
    delay(1000);
    Serial.print(".");
  }
  #endif 
}

void wifiInit() {
  String ssid = cfg.getString(CONFKEYS::KSSID, "");
  if (!WiFi.isConnected() && isWifiEnable() && ssid.length() > 0) {
    wifiConnect();
  }
  if(WiFi.isConnected()) {
    Serial.print("-->[WIFI] device network IP\t: ");
    Serial.println(WiFi.localIP());
    Serial.println("-->[WIFI] publish interval \t: " + String(stime * 2) + " sec.");
    ensureNtpSync();
    otaInit();
    wifiCloudsInit();
  }
}

void wifiStop() {
  if (WiFi.isConnected()) {
    WiFi.disconnect(true);
    delay(100);
    Serial.print("-->[WIFI] disconnecting:  \t: ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "failed" : "done");
  }
}

void wifiRestart() {
  wifiStop();
  wifiInit();
}

void wifiLoop() {
  static uint_least64_t wifiTimeStamp = 0;
  if (millis() - wifiTimeStamp > 10000) {
    wifiTimeStamp = millis();
    if (new_wifi && saveCLIWiFi()) {
        Serial.println("-->[WIFI] WiFi connected: " + WiFi.SSID());
    }
    setWifiConnected(WiFi.isConnected());
    String ssid = cfg.getString(CONFKEYS::KSSID, "");
    if (isWifiEnable() && ssid.length() > 0 && !WiFi.isConnected()) {
      wifiInit();
    }
    else if (!isWifiEnable() && WiFi.isConnected()) {
      wifiStop();
    }
    if (!WiFi.isConnected()) return;
    ensureNtpSync();
    influxDbInit();
    influxDbLoop();  // influxDB publication
    if (cfg.getBool(CONFKEYS::KANAIRE, false)) anaireLoop();
    if (cfg.getBool(CONFKEYS::KHOMEAS, false)) hassLoop();
  }
}

int getWifiRSSI() {
  if (WiFi.isConnected())
    return WiFi.RSSI();
  else
    return 0;
}
/**
 * @brief get the general info on reduced width for TFT screens and CLI.
*/
String getDeviceInfo() {
  String info = getHostId() + "\r\n";
  info = info + "Rev" + String(REVISION) + " v" + String(VERSION) + "\r\n";
  info = info + "" + getStationName() + "\r\n";
  info = info + String(FLAVOR) + "\r\n";
  info = info + "IP: " + WiFi.localIP().toString() + "\r\n";
  info = info + "OTA: " + String(TARGET) + " channel\r\n";
  info = info + "==================\r\n";
  info = info + "MEM: " + String(ESP.getFreeHeap() / 1024) + "Kb\r\n";
  info = info + "GUI: " + String(gui.getStackFree() / 1024) + "Kb\r\n";
  #ifdef CONFIG_IDF_TARGET_ESP32S3
  info = info + "CPU: " + String(powerESP32TempRead()) + "°C\r\n";
  #endif
  #ifndef DISABLE_BATT
  String charge = battery.isCharging() ? "charging" : "discharging";
  info = info + "BAT: " + String(battery.getVoltage()) + "v "+String(battery.getCharge()) +"% ("+charge+")\r\n";
  #endif
  return info;
}

void printWifiRSSI(){
  if (devmode) Serial.println("-->[WIFI] AP RSSI signal \t: " + String(getWifiRSSI()) + " dBm");
}
