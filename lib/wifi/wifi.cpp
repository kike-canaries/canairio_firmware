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

void wifiConnect(const char* ssid, const char* pass) {
  Serial.print("-->[WIFI] connecting to wifi\t: ");
  Serial.print(ssid);
  int wifi_retry = 0;
  WiFi.begin(ssid, pass);

  if (FAMILY == "ESP32-C3") WiFi.setTxPower(WIFI_POWER_8_5dBm);

  while (!WiFi.isConnected() && wifi_retry++ <= WIFI_RETRY_CONNECTION) {
    Serial.print(".");
    delay(500);  // increment this delay on possible reconnect issues
  }
  delay(500);
  if (WiFi.isConnected()) {
    isNewWifi = false;  // flag for config via BLE
    #ifndef DISABLE_CLI
    if(!wcli.isSSIDSaved(ssid))wcli.saveNetwork(ssid, pass);
    #endif
    Serial.println(" done."); 
  } else {
    Serial.println("fail!\r\n[E][WIFI] disconnected!");
  }
}

void wifiInit() {
  if (!WiFi.isConnected() && isWifiEnable() && ssid.length() > 0) {
    wifiConnect(ssid.c_str(), pass.c_str());
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
    Serial.println("-->[WIFI] Disconnecting..");
    WiFi.disconnect(true);
    delay(100);
  }
}

void wifiRestart() {
  wifiStop();
  wifiInit();
}

void wifiLoop() {
  static uint_least64_t wifiTimeStamp = 0;
  if (millis() - wifiTimeStamp > 5000) {
    wifiTimeStamp = millis();
    setWifiConnected(WiFi.isConnected());
    if (isWifiEnable() && ssid.length() > 0 && !WiFi.isConnected()) {
      wifiInit();
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
  info = info + "BAT: " + String(battery.getVoltage()) + "v "+String(battery.getCharge()) +"%\r\n";
  #endif
  return info;
}

void printWifiRSSI(){
  if (devmode) Serial.println("-->[WIFI] AP RSSI signal \t: " + String(getWifiRSSI()) + " dBm");
}