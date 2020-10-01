/**
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018 - 2020
 * @brief Particle meter sensor on ESP32 with bluetooth GATT notify server
 * @license GPL3
 */

#include <Arduino.h>

#include <Watchdog.hpp>
#include <ConfigApp.hpp>
#include <GUIUtils.hpp>
#include <Sensors.hpp>
#include <battery.hpp>
#include <bluetooth.hpp>
#include <wifi.hpp>

void displayGUI() {
    static uint_fast64_t timeStampGUI = 0;   // timestamp for GUI refresh
    if ((millis() - timeStampGUI > 1000)) {  // it should be minor than sensor loop
        timeStampGUI = millis();
        gui.pageStart();
        gui.displaySensorAverage(sensors.getPM25());
        gui.displaySensorData(
            sensors.getPM25(),
            sensors.getPM10(),
            getChargeLevel(),
            sensors.getHumidity(),
            sensors.getTemperature(),
            getWifiRSSI());
        gui.displayStatus(WiFi.isConnected(), true, bleIsConnected());
        gui.pageEnd();
    }
}

void onSensorDataOk() {
    gui.displaySensorLiveIcon();  // all sensors read are ok
}

/******************************************************************************
*  M A I N
******************************************************************************/

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n== CanAirIO Setup ==\n");
    pinMode(BUILTIN_LED, OUTPUT);
    // init graphic user interface
    gui.displayInit();
    gui.showWelcome();
    // init app preferences and load settings
    cfg.init("canairio");
    // device wifi macaddress and firmware version
    Serial.println("-->[INFO] ESP32MAC: " + String(cfg.deviceId));
    Serial.println("-->[INFO] Firmware " + gui.getFirmwareVersionCode());
    // init all sensors
    gui.welcomeAddMessage("Detecting sensors..");
    sensors.setOnDataCallBack(&onSensorDataOk);  // all data read callback
    sensors.setSampleTime(cfg.stime);            // config sensors sample time
    sensors.init();                              // start all sensors
    if(sensors.isPmSensorConfigured())
        gui.welcomeRepeatMessage(sensors.getPmDeviceSelected());
    else 
        gui.welcomeRepeatMessage("Detection !FAIL!");
    delay(500);
    // init battery (only for some boards)
    batteryInit();
    // Bluetooth low energy init (GATT server for device config)
    bleServerInit();
    gui.welcomeAddMessage("Bluetooth ready.");
    // WiFi and cloud communication
    wifiInit();
    if (WiFi.isConnected())
        gui.welcomeAddMessage("WiFi:" + cfg.ssid);
    else
        gui.welcomeAddMessage("WiFi: disabled.");
    Serial.println("-->[INFO] InfluxDb API:\t" + String(cfg.isIfxEnable()));
    Serial.println("-->[INFO] CanAirIO API:\t" + String(cfg.isApiEnable()));
    influxDbInit();
    apiInit();  // DEPRECATED
    if (WiFi.isConnected()) gui.welcomeAddMessage("API clouds ready.");
    // init watchdog timer for reboot in any loop blocker
    wd.init();
    gui.welcomeAddMessage("==SETUP READY==");
    delay(4000);
    displayGUI();
}

void loop() {
    sensors.loop();  // read sensor data and showed it
    batteryloop();   // battery charge status
    bleLoop();       // notify data to connected devices
    wifiLoop();      // check wifi and reconnect it
    apiLoop();       // CanAir.io API !! D E P R E C A T E D !!
    influxDbLoop();  // influxDB publication
    otaLoop();       // check for firmware updates
    wd.loop();       // watchdog for check loop blockers
    displayGUI();    // run GUI page
}