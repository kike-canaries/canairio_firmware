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
  if (!(wcli.getCurrentSSID().compareTo(ssid)==0)){
    saveWifi(ssid, pass);
    return;
  }
  Serial.print("-->[WIFI] connecting to wifi\t: ");
  Serial.print(ssid);
  wcli.wifiAPConnect(false);

  if (WiFi.isConnected()) {
    Serial.println(" done."); 
  } 
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
    if (new_wifi) saveCLIWiFi();
    setWifiConnected(WiFi.isConnected());
    String ssid = cfg.getString(CONFKEYS::KSSID, "");
    if (isWifiEnable() && ssid.length() > 0 && !WiFi.isConnected()) {
      wifiInit();
    }
    else if (!isWifiEnable() && WiFi.isConnected()) {
      wifiStop();
    }
    if (!WiFi.isConnected()) return;
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