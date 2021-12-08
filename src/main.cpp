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
#include <GUILib.hpp>
#include <Sensors.hpp>
#include <bluetooth.hpp>
#include <wifi.hpp>

uint16_t getMainValue() {
    uint16_t mainValue = 0;
    if (sensors.getMainDeviceSelected().isEmpty()) {
        mainValue = getPaxCount();
    } else if (sensors.getMainSensorTypeSelected() == Sensors::SENSOR_PM) {
        mainValue = sensors.getPM25();
    } else if (sensors.getMainSensorTypeSelected() == Sensors::SENSOR_CO2) {
        mainValue = sensors.getCO2();
    }
    return mainValue;
}

void refreshGUIData() {
    gui.displaySensorLiveIcon();  // all sensors read are ok
    
    float humi = sensors.getHumidity();
    if (humi == 0.0) humi = sensors.getCO2humi();

    float temp = sensors.getTemperature();
    if (temp == 0.0) temp = sensors.getCO2temp();  // TODO: temp could be 0.0

    gui.setSensorData(
        getMainValue(),
        humi,
        temp,
        getWifiRSSI(),
        sensors.getMainSensorTypeSelected());

    gui.setInfoData(getDeviceInfo());
}

class MyGUIUserPreferencesCallbacks : public GUIUserPreferencesCallbacks {
    void onWifiMode(bool enable){
        Serial.println("-->[MAIN] onWifi changed: "+String(enable));
        cfg.wifiEnable(enable);
        cfg.reload();
        if (!enable) wifiStop();
    };
    void onPaxMode(bool enable){
        Serial.println("-->[MAIN] onPax changed: "+String(enable));
        cfg.paxEnable(enable);
        cfg.reload();
    };
    void onBrightness(int value){
        Serial.println("-->[MAIN] onBrightness changed: "+String(value));
        cfg.saveBrightness(value);
    };
    void onColorsInverted(bool enable){
        Serial.println("-->[MAIN] onColorsInverted changed: "+String(enable));
        cfg.colorsInvertedEnable(enable);
    };
    void onSampleTime(int time){
        if(sensors.sample_time != time) {
            Serial.println("-->[MAIN] onSampleTime changed: "+String(time));
            cfg.saveSampleTime(time);
            cfg.reload();
            bleServerConfigRefresh();
            sensors.setSampleTime(cfg.stime);
        } 
    };
    void onCalibrationReady(){
        Serial.println("-->[MAIN] onCalibrationReady");
        sensors.setCO2RecalibrationFactor(400);   // ==> Calibration factor on outdoors
    };
};

class MyRemoteConfigCallBacks : public RemoteConfigCallbacks {
    void onCO2Calibration () {
        Serial.println("-->[MAIN] onRemoteConfig CO2 Calibration");
        sensors.setCO2RecalibrationFactor(400);   // ==> Calibration factor on outdoors
    };

    void onAltitudeOffset (float altitude) {
        Serial.println("-->[MAIN] onRemoteConfig new Altitude Offset");
        sensors.setCO2AltitudeOffset(altitude);
    };
};

/// sensors data callback
void onSensorDataOk() {
    log_i("[MAIN] onSensorDataOk");
    refreshGUIData();
}

/// sensors error callback
void onSensorDataError(const char * msg){
    log_w("[MAIN] onSensorDataError %s", msg);
    refreshGUIData();
}

void startingSensors() {
    Serial.println("-->[INFO] PM sensor configured: "+String(cfg.stype));
    gui.welcomeAddMessage("Detected sensor:");
    sensors.setOnDataCallBack(&onSensorDataOk);     // all data read callback
    sensors.setOnErrorCallBack(&onSensorDataError); // on data error callback
    sensors.setSampleTime(1);                       // sample time only for first use
    sensors.setTempOffset(cfg.toffset);             // temperature compensation
    sensors.setCO2AltitudeOffset(cfg.altoffset);    // CO2 altitude compensation
    sensors.detectI2COnly(cfg.i2conly);             // force only i2c sensors
    sensors.setDebugMode(cfg.devmode);              // debugging mode 
    sensors.init(cfg.getSensorType());              // start all sensors and
                                                    // The UART sensor is choosed on Android app.
                                                    // For more information about the supported sensors,
                                                    // please see the canairio_sensorlib documentation.

    if(!sensors.getMainDeviceSelected().isEmpty()) {
        Serial.print("-->[INFO] PM/CO2 sensor detected: ");
        Serial.println(sensors.getMainDeviceSelected());
        gui.welcomeAddMessage(sensors.getMainDeviceSelected());
    }
    else {
        Serial.println("-->[INFO] Detection sensors FAIL!");
        gui.welcomeAddMessage("Detection !FAILED!");
    }
}

/******************************************************************************
*  M A I N
******************************************************************************/

void setup() {
    Serial.begin(115200);
    delay(400);
    Serial.println("\n== CanAirIO Setup ==\n");

    // init app preferences and load settings
    cfg.init("canairio");

    // init graphic user interface
    gui.setBrightness(cfg.getBrightness());
    gui.setWifiMode(cfg.isWifiEnable());
    gui.setPaxMode(cfg.isPaxEnable());
    gui.setSampleTime(cfg.stime);
    gui.displayInit();
    gui.setCallbacks(new MyGUIUserPreferencesCallbacks());
    gui.showWelcome();

    // device wifi mac addres and firmware version
    Serial.println("-->[INFO] ESP32MAC: " + cfg.deviceId);
    Serial.println("-->[INFO] Hostname: " + getHostId());
    Serial.println("-->[INFO] Revision: " + gui.getFirmwareVersionCode());
    Serial.println("-->[INFO] Firmware: " + String(VERSION));
    Serial.println("-->[INFO] Flavor  : " + String(FLAVOR));
    Serial.println("-->[INFO] Target  : " + String(TARGET));

    // init all sensors
    Serial.println("-->[INFO] Detecting sensors..");
    pinMode(PMS_EN, OUTPUT);
    digitalWrite(PMS_EN, HIGH);
    startingSensors();
    // Setting callback for remote commands via Bluetooth config
    cfg.setRemoteConfigCallbacks(new MyRemoteConfigCallBacks());

    // init watchdog timer for reboot in any loop blocker
    wd.init();
    
        // WiFi and cloud communication
    wifiInit();
    Serial.printf("-->[INFO] InfluxDb:\t %s\n", cfg.isIfxEnable()  ? "enabled" : "disabled");
    Serial.printf("-->[INFO] WiFi    :\t %s\n", cfg.isWifiEnable() ? "enabled" : "disabled");
    gui.welcomeAddMessage("WiFi: "+String(cfg.isIfxEnable() ? "On" : "Off"));
    gui.welcomeAddMessage("Influx: "+String(cfg.isIfxEnable() ? "On" : "Off"));
 
    // Bluetooth low energy init (GATT server for device config)
    bleServerInit();
    gui.welcomeAddMessage("Bluetooth ready.");

    // wifi status 
    if (WiFi.isConnected())
        gui.welcomeAddMessage("WiFi:" + cfg.ssid);
    else
        gui.welcomeAddMessage("WiFi: disabled.");

    // sensor sample time and publish time (2x)
    gui.welcomeAddMessage("stime: "+String(cfg.stime)+ " sec.");
    gui.welcomeAddMessage(cfg.getDeviceId());   // mac address
    gui.welcomeAddMessage("Watchdog:"+String(WATCHDOG_TIME));
    gui.welcomeAddMessage("==SETUP READY==");
    delay(500);
    gui.showMain();
    refreshGUIData();
    delay(600);
    sensors.loop();
    sensors.setSampleTime(cfg.stime);        // config sensors sample time (first use)
}

void loop() {
    sensors.loop();  // read sensor data and showed it
    bleLoop();       // notify data to connected devices
    snifferLoop();   // pax counter calc (only when WiFi is Off)
    wifiLoop();      // check wifi and reconnect it
    otaLoop();       // check for firmware updates
    wd.loop();       // watchdog for check loop blockers
                     // update GUI flags:
    gui.setGUIStatusFlags(WiFi.isConnected(), true, bleIsConnected());
    gui.loop();
}
