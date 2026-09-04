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

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  Serial.print("-->[WIFI] NTP sync ok. Time now\t: ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

bool isTimeValid(time_t t) {
  return t > 1609459200;  // 2021-01-01T00:00:00Z
}

static bool getTimezoneOffsetFromGeo(int32_t &offsetSeconds) {
  static int32_t cachedOffset = INT32_MIN;
  static String cachedGeo = "";

  double lat = cfg.getDouble("lat", 0.0);
  double lon = cfg.getDouble("lon", 0.0);
  String geo = cfg.getString("geo", "");

  if ((lat == 0.0 || lon == 0.0) && geo.length() > 5) {
    if (geo != cachedGeo || cachedOffset == INT32_MIN) {
      Geohash gh;
      float tlat = 0.0f;
      float tlon = 0.0f;
      gh.decode(geo.c_str(), geo.length(), &tlon, &tlat);
      lat = tlat;
      lon = tlon;
      int32_t h = (int32_t)lround(lon / 15.0);
      if (h < -12) h = -12;
      if (h > 14) h = 14;
      cachedOffset = h * 3600;
      cachedGeo = geo;
    }
    offsetSeconds = cachedOffset;
    return true;
  }

  if (lat == 0.0 || lon == 0.0) return false;

  int32_t offsetHours = (int32_t)lround(lon / 15.0);
  if (offsetHours < -12) offsetHours = -12;
  if (offsetHours > 14) offsetHours = 14;
  offsetSeconds = offsetHours * 3600;
  return true;
}

void ensureNtpSync(bool sync_now) {
  if (!WiFi.isConnected()) return;

  static bool ntpConfigured = false;
  static bool ntpSynced = false;
  static uint32_t lastAttemptMs = 0;
  static int32_t lastOffsetSeconds = INT32_MIN;

  if (ntpSynced && isTimeValid(time(nullptr))) return;
  if (!sync_now && (millis() - lastAttemptMs < 60000)) return;  // retry at most every 60s
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
  }

  time_t now = time(nullptr);
  if (isTimeValid(now)) {
    ntpSynced = true;
    printLocalTime();
  } 
}

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
  else {
    log_i("wifiConnect was skipped");
    log_i("isConnect: %i isWiFiEnable: %i ssidLenght: %i\r\n", WiFi.isConnected(), isWifiEnable(), ssid.length());
  }
  if(WiFi.isConnected()) {
    Serial.print("-->[WIFI] device network IP\t: ");
    Serial.println(WiFi.localIP());
    Serial.println("-->[WIFI] publish interval \t: " + String(stime * 2) + " sec.");

    String sname = !(cfg.getString("geo", "")).isEmpty() ? getStationName() : "not configured :(\tRun \"sgeoh\" command ;)";
    Serial.printf("-->[INFO] CanAirIO station name\t: %s\r\n", sname.c_str());

    ensureNtpSync(true);
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
    ensureNtpSync(false);
    influxDbInit();
    influxDbLoop();  // influxDB publication
    if (!ota.isConfigured()) otaInit();
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
String getDeviceInfo(bool isCLI) {
  String info = getHostId() + "\r\n";
  info = info + "" + getStationName() + "\r\n";
  info = info + String(FLAVOR) + "\r\n";
  if (isCLI) info = info + getVersion() + " (" + getGitVersion() + ")\r\n";
  else info = info + "Rev: " + String(REVISION) + " v" + String(VERSION) + "\r\n";

  info = info + "===================\r\n";
  if (!isCLI) info = info + "IP: " + WiFi.localIP().toString() + "\r\n";
  info = info + "OTA: " + String(TARGET) + " channel\r\n";
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char strftime_buf[64];
    if (isCLI) strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    else strftime(strftime_buf, sizeof(strftime_buf), "%a %d %H:%M", &timeinfo);
    info = info + "NTP: " + String(strftime_buf) + "\r\n";
  }

  #ifdef CONFIG_IDF_TARGET_ESP32S3
  info = info + "CPU: " + String(powerESP32TempRead()) + "°C\r\n";
  #endif
  #ifndef DISABLE_BATT
  String charge = battery.isCharging() ? "charging" : "discharging";
  if (isCLI) info = info + "BAT: " + String(battery.getVoltage()) + "v "+String(battery.getCharge()) +"% ("+charge+")\r\n";
  else info = info + "BAT: " + String(battery.getVoltage()) + "v "+String(battery.getCharge()) +"%\r\n";
  #endif
  return info;
}

void printWifiRSSI(){
  if (devmode) Serial.println("-->[WIFI] AP RSSI signal \t: " + String(getWifiRSSI()) + " dBm");
}
