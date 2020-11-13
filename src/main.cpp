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

/// sensors data callback
void onSensorDataOk() {
    gui.displaySensorLiveIcon();  // all sensors read are ok
}

/// sensors error callback
void onSensorDataError(const char * msg){
    Serial.println(msg);
}

void startingSensors() {
    Serial.println("-->[INFO] PM sensor configured: "+String(cfg.stype));
    gui.welcomeAddMessage("Detected sensor:");
    sensors.setOnDataCallBack(&onSensorDataOk);   // all data read callback
    sensors.setSampleTime(cfg.stime);             // config sensors sample time
    sensors.setDebugMode(false);                  // [optional] debug mode
    sensors.init(cfg.getSensorType());            // start all sensors and
                                                  // try to detect configured PM sensor.
                                                  // Sensors supported: Panasonic, Honeywell, Plantower and Sensirion
                                                  // The configured sensor is choosed on Android app.
                                                  // For more information about the supported sensors,
                                                  // please see the canairio_sensorlib documentation.

    if(sensors.isPmSensorConfigured()){
        Serial.print("-->[INFO] PM sensor detected: ");
        Serial.println(sensors.getPmDeviceSelected());
        gui.welcomeAddMessage(sensors.getPmDeviceSelected());
    }
    else {
        Serial.println("-->[INFO] Detection sensors FAIL!");
        gui.welcomeAddMessage("Detection !FAILED!");
    }
}

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

/******************************************************************************
*  M A I N
******************************************************************************/

void setup() {
    Serial.begin(115200);
    delay(400);
    Serial.println("\n== CanAirIO Setup ==\n");
    pinMode(BUILTIN_LED, OUTPUT);

    // init graphic user interface
    gui.displayInit();
    gui.showWelcome();

    // init app preferences and load settings
    // cfg.setDebugMode(true);
    cfg.init("canairio");

    // device wifi mac addres and firmware version
    Serial.println("-->[INFO] ESP32MAC: " + cfg.deviceId);
    Serial.println("-->[INFO] Firmware " + gui.getFirmwareVersionCode());

    // init all sensors
    Serial.println("-->[INFO] Detecting sensors..");
    startingSensors();

    // init battery (only for some boards)
    batteryInit();

    // Bluetooth low energy init (GATT server for device config)
    bleServerInit();
    gui.welcomeAddMessage("Bluetooth ready.");

    // WiFi and cloud communication
    wifiInit();
    Serial.println("-->[INFO] InfluxDb API:\t" + String(cfg.isIfxEnable()));
    Serial.println("-->[INFO] CanAirIO API:\t" + String(cfg.isApiEnable()));
    gui.welcomeAddMessage("CanAirIO API:"+String(cfg.isApiEnable()));
    gui.welcomeAddMessage("InfluxDb :"+String(cfg.isIfxEnable()));
    influxDbInit();
    apiInit();  // DEPRECATED

    // init watchdog timer for reboot in any loop blocker
    wd.init();
    gui.welcomeAddMessage("Watchdog ready");

    // mac address
    gui.welcomeAddMessage(cfg.getDeviceId());

    // wifi status 
    if (WiFi.isConnected())
        gui.welcomeAddMessage("WiFi:" + cfg.ssid);
    else
        gui.welcomeAddMessage("WiFi: disabled.");

    // sensor sample time and publish time (2x)
    gui.welcomeAddMessage("stime: "+String(cfg.stime)+ " sec.");

    gui.welcomeAddMessage("==SETUP READY==");
    delay(1000);

    // display main screen
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