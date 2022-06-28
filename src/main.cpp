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
#include <power.hpp>
#include <wifi.hpp>

UNIT selectUnit = UNIT::NUNIT;
UNIT nextUnit = UNIT::NUNIT;
GUIData data;

AQI_COLOR selectAQIColor() {
    if (selectUnit == UNIT::PM25 || selectUnit == UNIT::PM10 || selectUnit == UNIT::PM1)
        return AQI_COLOR::AQI_PM;
    else if (selectUnit == UNIT::CO2)
        return AQI_COLOR::AQI_CO2;
    else
        return AQI_COLOR::AQI_NONE;
}

void loadGUIData() {
    float humi = sensors.getHumidity();
    if (humi == 0.0) humi = sensors.getCO2humi();

    float temp = sensors.getTemperature();
    if (temp == 0.0) temp = sensors.getCO2temp();  // TODO: temp could be 0.0

    data.temp = temp;
    data.humi = humi;
    data.rssi = getWifiRSSI();

    UNIT user_unit = (UNIT) cfg.getUnitSelected();
    if (selectUnit != user_unit && sensors.isUnitRegistered(user_unit)) {
        selectUnit = user_unit;
    }

    // Main unit selection
    if (sensors.getUnitsRegisteredCount() == 0 && cfg.isPaxEnable()) {
        data.mainValue = getPaxCount();
        data.unitName = "PAX";
        data.unitSymbol = "PAX";
        data.mainUnitId = UNIT::NUNIT;
        data.color = AQI_COLOR::AQI_PM;
    } else if (selectUnit != UNIT::NUNIT) {
        if (selectUnit != RADIATION){
           data.mainValue = sensors.getUnitValue(selectUnit);
           data.unitName = sensors.getUnitName(selectUnit);
           data.unitSymbol = sensors.getUnitSymbol(selectUnit);
           }else{
           data.mainValue = 100 * sensors.getUnitValue(selectUnit); // from uSv/h to uSv/h*100
           data.unitName = "radiation";
           data.unitSymbol = "uSv/h *100";
           }
        data.mainUnitId = selectUnit;
        data.color = selectAQIColor();
    }
    // Minor unit selection
    if (nextUnit != UNIT::NUNIT) {
        if (nextUnit != RADIATION){
           data.minorValue = sensors.getUnitValue(nextUnit);
           data.unitName = sensors.getUnitName(nextUnit);
           data.unitSymbol = sensors.getUnitSymbol(nextUnit);
           }else{
           data.minorValue = 100 * sensors.getUnitValue(nextUnit); // from uSv/h to uSv/h*100
           data.unitName = "radiation";
           data.unitSymbol = "uSv/h *100";
           }
        data.color = selectAQIColor();
    }
    data.onSelectionUnit = nextUnit;
}

void refreshGUIData() {
    loadGUIData();
    gui.displaySensorLiveIcon();  // all sensors read are ok 
    gui.setSensorData(data);
    gui.setInfoData(getDeviceInfo());
    logMemory ("LOOP");
}

class MyGUIUserPreferencesCallbacks : public GUIUserPreferencesCallbacks {
    void onWifiMode(bool enable) {
        Serial.println("-->[MAIN] Wifi enable changed\t: " + String(enable));
        cfg.wifiEnable(enable);
        cfg.reload();
        if (!enable) wifiStop();
    };
    void onPaxMode(bool enable) {
        Serial.println("-->[MAIN] onPax enable changed\t: " + String(enable));
        cfg.paxEnable(enable);
        cfg.reload();
    };
    void onBrightness(int value) {
        Serial.println("-->[MAIN] onBrightness changed\t: " + String(value));
        cfg.saveBrightness(value);
    };
    void onColorsInverted(bool enable) {
        Serial.println("-->[MAIN] onColors changed    \t: " + String(enable));
        cfg.colorsInvertedEnable(enable);
    };
    void onSampleTime(int time) {
        if (sensors.sample_time != time) {
            Serial.println("-->[MAIN] onSampleTime changed\t: " + String(time));
            cfg.saveSampleTime(time);
            cfg.reload();
            bleServerConfigRefresh();
            sensors.setSampleTime(cfg.stime);
        }
    };
    void onCalibrationReady() {
        Serial.println("-->[MAIN] onCalibrationReady");
        sensors.setCO2RecalibrationFactor(400);  // ==> Calibration factor on outdoors
    };
    void onUnitSelectionToggle() {
        Serial.println("-->[MAIN] onUnitSelectionToggle");
        nextUnit = sensors.getNextUnit();
        if (nextUnit == UNIT::NUNIT ) {
            nextUnit = sensors.getNextUnit();
        }
        refreshGUIData();
    };
    void onUnitSelectionConfirm() {
        Serial.print("-->[MAIN] Unit selected  \t: ");
        if (nextUnit!=UNIT::NUNIT && nextUnit!=selectUnit) {
            Serial.println(sensors.getUnitName(nextUnit));
            selectUnit = nextUnit;
            cfg.saveUnitSelected(selectUnit);
        } else {
            Serial.println("NONE");
        }
    };
    void onPowerOff(){
        powerDeepSleepButton();
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

    void onSeaLevelPressure (float hpa) {
        Serial.println("-->[MAIN] onRemoteConfig new Sea Level Pressure");
        sensors.setSeaLevelPressure(hpa);
    }
};

class MyBatteryUpdateCallbacks : public BatteryUpdateCallbacks {
    void onBatteryUpdate(float voltage, int charge, bool charging) {
        gui.setBatteryStatus(voltage, charge, charging);
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

void printSensorsDetected() {
    Serial.println("-->[INFO] Sensors detected\t: " + String(sensors.getSensorsRegisteredCount()));
    gui.welcomeAddMessage("Sensors: " + String(sensors.getSensorsRegisteredCount()));
    int i = 0;
    while (sensors.getSensorsRegistered()[i++] != 0) {
        gui.welcomeAddMessage(sensors.getSensorName((SENSORS)sensors.getSensorsRegistered()[i - 1]));
    }
}

void startingSensors() {
    Serial.println("-->[INFO] config UART sensor\t: "+sensors.getSensorName((SENSORS)cfg.stype));
    gui.welcomeAddMessage("Enabling sensors:");
    sensors.setOnDataCallBack(&onSensorDataOk);     // all data read callback
    sensors.setOnErrorCallBack(&onSensorDataError); // on data error callback
    sensors.setSampleTime(cfg.stime);               // config sensors sample time (first use)
    sensors.setTempOffset(cfg.toffset);             // temperature compensation
    sensors.setCO2AltitudeOffset(cfg.altoffset);    // CO2 altitude compensation
    sensors.detectI2COnly(cfg.i2conly);             // force only i2c sensors
    sensors.setDebugMode(cfg.devmode);              // debugging mode 
    sensors.init(cfg.getSensorType());              // start all sensors and
                                                    // The UART sensor is choosed on Android app.
                                                    // For more information about the supported sensors,
                                                    // please see the canairio_sensorlib documentation.
    if(sensors.getSensorsRegisteredCount()==0){
        Serial.println("-->[INFO] Main sensors detected\t: 0");
        gui.welcomeAddMessage("Not sensors detected");
        gui.welcomeAddMessage("Default: PAX");
    }
    else{
        printSensorsDetected();    
    }

    Serial.printf("-->[INFO] registered units\t:\n");
    delay(1000);
    sensors.readAllSensors();                       // only to force to register all sensors
    gui.welcomeAddMessage("Units count: "+String(sensors.getUnitsRegisteredCount()));
    selectUnit = (UNIT) cfg.getUnitSelected();
    Serial.printf("-->[INFO] restored saved unit \t: %s\n",sensors.getUnitName(selectUnit).c_str());
    if (!sensors.isUnitRegistered(selectUnit)){
        sensors.resetNextUnit();
        selectUnit = sensors.getNextUnit();  // auto selection of sensor unit to show
        Serial.printf("-->[INFO] not found! set to\t: %s\n",sensors.getUnitName(selectUnit).c_str());
    }
    gui.welcomeAddMessage("Show unit: "+sensors.getUnitName(selectUnit));
    sensors.printUnitsRegistered(true);
    delay(300);
}

/******************************************************************************
*  M A I N
******************************************************************************/

void setup() {
    Serial.begin(115200);
    delay(400);
    Serial.println("\n== CanAirIO Setup ==\n");
    logMemory("INIT");

    // init app preferences and load settings
    cfg.init("canairio");
    logMemory("CONF");
    battery.setUpdateCallbacks(new MyBatteryUpdateCallbacks());
    battery.init(cfg.devmode);
    battery.update();
    powerInit();

    // init graphic user interface
    gui.setBrightness(cfg.getBrightness());
    gui.setWifiMode(cfg.isWifiEnable());
    gui.setPaxMode(cfg.isPaxEnable());
    gui.setSampleTime(cfg.stime);
    gui.displayInit();
    gui.setCallbacks(new MyGUIUserPreferencesCallbacks());
    gui.showWelcome();
    logMemory("GLIB");

    // device wifi mac addres and firmware version
    Serial.println("-->[INFO] ESP32MAC\t\t: " + cfg.deviceId);
    Serial.println("-->[INFO] Hostname\t\t: " + getHostId());
    Serial.println("-->[INFO] Revision\t\t: " + gui.getFirmwareVersionCode());
    Serial.println("-->[INFO] Firmware\t\t: " + String(VERSION));
    Serial.println("-->[INFO] Flavor  \t\t: " + String(FLAVOR));
    Serial.println("-->[INFO] Target  \t\t: " + String(TARGET));
    Serial.println("-->[INFO] == Detecting Sensors ==");
    Serial.println("-->[INFO] Sensorslib version\t: " + sensors.getLibraryVersion());
    Serial.println("-->[INFO] enable sensor GPIO\t: " + String(MAIN_HW_EN_PIN));
    logMemory("GPIO");
    startingSensors();
    logMemory("SLIB");
    // Setting callback for remote commands via Bluetooth config
    cfg.setRemoteConfigCallbacks(new MyRemoteConfigCallBacks());
    // init watchdog timer for reboot in any loop blocker
    wd.init();
    // WiFi and cloud communication
    logMemory("WDOG");
    gui.welcomeAddMessage("Connecting..");
    wifiInit();
    logMemory("WIFI");
    Serial.printf("-->[INFO] InfluxDb cloud \t: %s\n", cfg.isIfxEnable()  ? "enabled" : "disabled");
    Serial.printf("-->[INFO] WiFi current config\t: %s\n", cfg.isWifiEnable() ? "enabled" : "disabled");
    gui.welcomeAddMessage("WiFi: "+String(cfg.isIfxEnable() ? "On" : "Off"));
    gui.welcomeAddMessage("Influx: "+String(cfg.isIfxEnable() ? "On" : "Off"));
 
    // Bluetooth low energy init (GATT server for device config)
    bleServerInit();
    logMemory("BLE ");
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
    delay(2000);
    gui.showMain();
    refreshGUIData();
    logMemory("GLIB");
    Serial.printf("-->[INFO] sensors units detected\t: %d\n", sensors.getUnitsRegisteredCount());
    Serial.printf("-->[INFO] unit selected to show \t: %s\n",sensors.getUnitName(selectUnit).c_str());
    Serial.printf("-->[HEAP] sizeof sensors\t: %04ub\n", sizeof(sensors));
    Serial.printf("-->[HEAP] sizeof config \t: %04ub\n", sizeof(cfg));
    Serial.printf("-->[HEAP] sizeof GUI    \t: %04ub\n", sizeof(gui));
    Serial.println("\n==>[INFO] Setup End ===\n");
}

void loop() {
    sensors.loop();  // read sensor data and showed it
    bleLoop();       // notify data to connected devices
    otaLoop();       // check for firmware updates
    snifferLoop();   // pax counter calc (only when WiFi is Off)
    wifiLoop();      // check wifi and reconnect it
    wd.loop();       // watchdog for check loop blockers
                     // update GUI flags:
    gui.setGUIStatusFlags(WiFi.isConnected(), true, bleIsConnected());
    gui.loop();

    battery.loop();  // refresh battery level and voltage
    powerLoop();     // check power status and manage power saving
}
