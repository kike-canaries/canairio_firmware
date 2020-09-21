/**
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018 - 2020
 * @brief Particle meter sensor on ESP32 with bluetooth GATT notify server
 * @license GPL3
 */

#include <Arduino.h>
#include <ConfigApp.hpp>
#include <Sensors.hpp>
#include <GUIUtils.hpp>
#include <bluetooth.hpp>
#include <wifi.hpp>
#include <watchdog.hpp>
#include <battery.hpp>

void showValues(){
  gui.displaySensorAverage(sensors.getPM25());
  gui.displaySensorData(
    sensors.getPM25(), 
    sensors.getPM10(), 
    getChargeLevel(), 
    sensors.getHumidity(), 
    sensors.getTemperature(),
    getWifiRSSI()
  );
  gui.displayLiveIcon();
}

/******************************************************************************
*  M A I N
******************************************************************************/

void setup(){
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n== INIT SETUP ==\n");
  pinMode(BUILTIN_LED,OUTPUT);
  gui.displayInit();
  gui.showWelcome();
  cfg.init("canairio");
  Serial.println("-->[INFO] ESP32MAC: "+String(cfg.deviceId));
  gui.welcomeAddMessage("Sensors test..");
  batteryInit();
  sensors.init();
  bleServerInit();
  gui.welcomeAddMessage("GATT server..");
  if(cfg.ssid.length()>0) gui.welcomeAddMessage("WiFi:"+cfg.ssid);
  else gui.welcomeAddMessage("WiFi radio test..");
  wifiInit();
  gui.welcomeAddMessage("CanAirIO API..");
  influxDbInit();
  apiInit();
  gui.welcomeAddMessage("==SETUP READY==");
  // watchdogInit();  // enable timer for reboot in any loop blocker
  delay(500);
}

void loop(){
  // gui.pageStart();
  sensors.loop();    // read sensor data and showed it
  batteryloop();   // battery charge status
  bleLoop();       // notify data to connected devices
  wifiLoop();      // check wifi and reconnect it
  apiLoop();       // CanAir.io API !! D E P R E C A T E D !!
  influxDbLoop();  // influxDB publication
  // statusLoop();    // update sensor status GUI
  otaLoop();       // check for firmware updates
  // gui.pageEnd();   // gui changes push
  // watchdogLoop();     // reset every 20 minutes with Wifion
}