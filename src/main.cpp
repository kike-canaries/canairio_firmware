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

// void showValues(int pm25, int pm10){
//   gui.displaySensorAverage(apm25); // it was calculated on bleLoop()
//   gui.displaySensorData(pm25, pm10, chargeLevel, humi, temp, rssi);
//   gui.displayLiveIcon();
//   saveDataForAverage(pm25, pm10);
//   WrongSerialData = false;
// }


/******************************************************************************
*  M A I N
******************************************************************************/

// void statusLoop(){
//   if () {
//     Serial.print("-->[STATUS] ");
//     Serial.println(status.to_string().c_str());
//     updateStatusError();
//     wifiCheck();
//   }
//   gui.updateError(getErrorCode());
//   gui.displayStatus(wifiOn,true,deviceConnected,dataSendToggle);
//   if(triggerSaveIcon++<3) gui.displayPrefSaveIcon(true);
//   else gui.displayPrefSaveIcon(false);
//   if(dataSendToggle) dataSendToggle=false;
// }

void setup(){
  Serial.begin(115200);
  gui.displayInit();
  gui.showWelcome();
  cfg.init("canairio");
  Serial.println("\n== INIT SETUP ==\n");
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
  pinMode(LED,OUTPUT);
  gui.welcomeAddMessage("==SETUP READY==");
  watchdogInit();  // enable timer for reboot in any loop blocker
  delay(500);
}

void loop(){
  gui.pageStart();
  sensors.loop();    // read sensor data and showed it
  batteryloop();   // battery charge status
  bleLoop();       // notify data to connected devices
  wifiLoop();      // check wifi and reconnect it
  apiLoop();       // CanAir.io API publication
  influxDbLoop();  // influxDB publication
  // statusLoop();    // update sensor status GUI
  otaLoop();       // check for firmware updates
  gui.pageEnd();   // gui changes push
  watchdogLoop();     // reset every 20 minutes with Wifion
}