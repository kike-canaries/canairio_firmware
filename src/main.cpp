/**
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018 - 2025
 * @brief Particle meter sensor on ESP32 with bluetooth GATT notify server
 * @license GPL3
 */

#include <Arduino.h>
#include <Sensors.hpp>
#include <MQTT.h>
#include "ConfigApp.hpp"
#include "GUILib.hpp"
#include "Watchdog.hpp"
#include "OTAHandler.h"
#include "power.hpp"
#include "wifi.hpp"
#include "bluetooth.hpp"
#include "logmem.hpp"
#include "sniffer.h"

#ifndef DISABLE_CLI
#include "cli.hpp"
#endif

#ifdef LORADEVKIT
#include <lorawan.h>
#endif

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

    UNIT user_unit = (UNIT) getUnitSelected();
    if (selectUnit != user_unit && sensors.isUnitRegistered(user_unit)) {
        selectUnit = user_unit;
    }

    // Main unit selection
    if (sensors.getUnitsRegisteredCount() == 0 && isPaxEnable()) {
        data.mainValue = getPaxCount();
        data.unitName = "PAX";
        data.unitSymbol = "PAX";
        data.mainUnitId = UNIT::NUNIT;
        data.color = AQI_COLOR::AQI_PM;
    } else if (selectUnit != UNIT::NUNIT) {
        data.mainValue = sensors.getUnitValue(selectUnit);
        data.unitName = sensors.getUnitName(selectUnit);
        data.unitSymbol = sensors.getUnitSymbol(selectUnit);
        data.mainUnitId = selectUnit;
        data.color = selectAQIColor();
    }
    // Minor unit selection
    if (nextUnit != UNIT::NUNIT) {
        data.minorValue = sensors.getUnitValue(nextUnit);
        data.unitName = sensors.getUnitName(nextUnit);
        data.unitSymbol = sensors.getUnitSymbol(nextUnit);
        data.color = selectAQIColor();
    }
    data.onSelectionUnit = nextUnit;
}

void refreshGUIData(bool onUnitSelection) {
    loadGUIData();
    if (!onUnitSelection) {
        gui.displaySensorLiveIcon();  // all sensors read are ok 
        gui.setInfoData(getDeviceInfo());
        printWifiRSSI();
    }
    gui.setSensorData(data);
    logMemory ("LOOP");
}

class MyGUIUserPreferencesCallbacks : public GUIUserPreferencesCallbacks {
    void onWifiMode(bool enable) {
        Serial.println("-->[MAIN] Wifi enable changed\t: " + String(enable));
        wifiEnable(enable);
        reload();
        if (!enable) wifiStop();
    };
    void onPaxMode(bool enable) {
        Serial.println("-->[MAIN] onPax enable changed\t: " + String(enable));
        paxEnable(enable);
        reload();
    };
    void onBrightness(int value) {
        Serial.println("-->[MAIN] onBrightness changed\t: " + String(value));
        saveBrightness(value);
    };
    void onColorsInverted(bool enable) {
        Serial.println("-->[MAIN] onColors changed    \t: " + String(enable));
        colorsInvertedEnable(enable);
    };
    void onSampleTime(int time) {
        if (sensors.sample_time != time) {
            Serial.println("-->[MAIN] onSampleTime changed\t: " + String(time));
            saveSampleTime(time);
            reload();

#ifndef DISABLE_BLE
            bleServerConfigRefresh();
#endif
            sensors.setSampleTime(stime);
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
        refreshGUIData(true);
    };
    void onUnitSelectionConfirm() {
        Serial.print("-->[MAIN] Unit selected  \t: ");
        if (nextUnit!=UNIT::NUNIT && nextUnit!=selectUnit) {
            Serial.println(sensors.getUnitName(nextUnit));
            selectUnit = nextUnit;
            saveUnitSelected(selectUnit);
        } else {
            Serial.println("NONE");
        }
    };

    void onPowerOff(){
        if(cfg.getBool(CONFKEYS::KWKUPRST,false))
            powerCompleteShutdown();
        else
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

#ifndef DISABLE_BATT
class MyBatteryUpdateCallbacks : public BatteryUpdateCallbacks {
    void onBatteryUpdate(float voltage, int charge, bool charging) {
        gui.setBatteryStatus(voltage, charge, charging);
    };
};
#endif

/// sensors data callback
void onSensorDataOk() {
    log_i("[MAIN] onSensorDataOk");
    refreshGUIData(false);
}

/// sensors error callback
void onSensorDataError(const char * msg){
    log_w("[MAIN] onSensorDataError %s", msg);
    refreshGUIData(false);
}

void printSensorsDetected() {
    gui.welcomeAddMessage("Sensors: " + String(sensors.getSensorsRegisteredCount()));
    int i = 0;
    while (sensors.getSensorsRegistered()[i++] != 0) {
        gui.welcomeAddMessage(sensors.getSensorName((SENSORS)sensors.getSensorsRegistered()[i - 1]));
    }
}

void startingSensors() {
    Serial.println("-->[INFO] config UART sensor\t: "+sensors.getSensorName((SENSORS)stype));
    gui.welcomeAddMessage("Init sensors..");
    int geigerPin = cfg.getInt(CONFKEYS::KGEIGERP, -1);// Geiger sensor pin (config it via CLI) 
    int tunit = cfg.getInt(CONFKEYS::KTEMPUNT, 0);     // Temperature unit (defaulut celsius)
    bool i2conly = cfg.getBool(CONFKEYS::KI2CONLY, false);
    sensors.setOnDataCallBack(&onSensorDataOk);        // all data read callback
    sensors.setOnErrorCallBack(&onSensorDataError);    // on data error callback
    sensors.setDebugMode(devmode);                     // debugging mode 
    sensors.setSampleTime(stime);                      // config sensors sample time (first use)
    sensors.setTempOffset(toffset);                    // temperature compensation
    sensors.setCO2AltitudeOffset(altoffset);           // CO2 altitude compensation
    sensors.detectI2COnly(i2conly);                    // force only i2c sensors
    sensors.enableGeigerSensor(geigerPin);             // Geiger sensor init
    sensors.setTemperatureUnit((TEMPUNIT)tunit);       // Config temperature unit (K,C or F)
    int mUART = stype;                                 // optional UART sensor choosed on the Android app
    int mTX = sTX;                                     // UART TX defined via setup
    int mRX = sRX;                                     // UART RX defined via setup

    if (sTX == -1 && sRX == -1) {
        sensors.init(mUART);                        // start all sensors (board predefined pins)
    }
    else {
        sensors.init(mUART, mRX, mTX);              // start all sensors and custom pins via setup.
    }
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

    delay(500);
    sensors.readAllSensors();                       // only to force to register all sensors
    delay(500);
    sensors.readAllSensors();                       // only to force to register all sensors
    delay(10);
    gui.welcomeAddMessage("Units count: "+String(sensors.getUnitsRegisteredCount()));
    selectUnit = (UNIT) getUnitSelected();
    Serial.printf("-->[INFO] restored saved unit\t: %s\r\n",sensors.getUnitName(selectUnit).c_str());
    if (!sensors.isUnitRegistered(selectUnit)){
        sensors.resetNextUnit();
        selectUnit = sensors.getNextUnit();  // auto selection of sensor unit to show
        Serial.printf("-->[INFO] not found! set to\t: %s\r\n",sensors.getUnitName(selectUnit).c_str());
    }
    gui.welcomeAddMessage("Show unit: "+sensors.getUnitName(selectUnit));
    sensors.printUnitsRegistered(true);
    delay(300);
}

#if defined(TTGO_T7) || defined(TTGO_T7S3)
#define BATTERY_MIN_V 3.4
#define BATTERY_MAX_V 4.28
#define BATTCHARG_MIN_V 3.8
#define BATTCHARG_MAX_V 4.34
#else
#define BATTERY_MIN_V 3.1
#define BATTERY_MAX_V 4.04
#define BATTCHARG_MIN_V 4.06
#define BATTCHARG_MAX_V 4.198
#endif

void initBattery() {
#ifndef DISABLE_BATT
  if (strcmp(FAMILY, "ESP32-C3") != 0) {
    battery.setUpdateCallbacks(new MyBatteryUpdateCallbacks());
    battery.setBattLimits(
      cfg.getFloat(CONFKEYS::KBATVMI, BATT_MIN_V),
      cfg.getFloat(CONFKEYS::KBATVMX, BATT_MAX_V));
    battery.setChargLimits(
      cfg.getFloat(CONFKEYS::KCHRVMI, BCHARG_MIN_V),
      cfg.getFloat(CONFKEYS::KCHRVMX, BCHARG_MAX_V));
    if (devmode) battery.printLimits();
    battery.init(devmode);
    battery.update();
  }
#endif
}

void initCLIFailsafe() {
#ifndef DISABLE_CLI
  if (cfg.getBool(CONFKEYS::KFAILSAFE, true)) {
    delay(2000); // wait for new S3 and C3 CDC serial
    gui.welcomeAddMessage("wait for setup..");
    Serial.println("\n-->[INFO] == Type \"setup\" for enter in safe mode (over in 10seg!) ==");
    cliInit();
    logMemory("CLI ");
  }
#endif
}

void initCLI() {
#ifndef DISABLE_CLI
  Serial.println("\n==>[INFO] Setup end. CLI enable. Press ENTER  ===\r\n");
  if (!cfg.getBool(CONFKEYS::KFAILSAFE, true)) cliInit();
  cliTaskInit();
  logMemory("CLI ");
#else
  Serial.println("\n==>[INFO] Setup End. ===\r\n");
#endif
}

/******************************************************************************
*  M A I N
******************************************************************************/

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.flush();
    checkCoreDumpPartition();
    Serial.println("\n== CanAirIO Setup ==\r\n");
    logMemory("INIT");
    // init app preferences and load settings
    init("canairio");
    powerInit();
    Serial.setDebugOutput(devmode);
    logMemory("CONF"); 
    // init graphic user interface
    sensors.startI2C(); // I2C shared bus with OLED (ESP32S3 issue)
    gui.setBrightness(getBrightness());
    gui.setWifiMode(isWifiEnable());
    gui.setPaxMode(isPaxEnable());
    gui.setSampleTime(stime);
    gui.setEmoticons(cfg.getBool(CONFKEYS::KEMOTICO,true));
    gui.displayInit(cfg.getInt(CONFKEYS::KOLEDTYPE, 0));
    gui.flipVertical(cfg.getBool(CONFKEYS::KFLIPV,false));
    gui.setCallbacks(new MyGUIUserPreferencesCallbacks());
    gui.showWelcome();
    logMemory("GLIB");
    // CanAirIO CLI init and first setup (safe mode)
    initCLIFailsafe(); 
    // init battery monitor
    initBattery(); 
    logMemory("BATT");
    // device wifi mac addres and firmware version
    Serial.println("-->[INFO] ESP32MAC\t\t: " + deviceId);
    Serial.println("-->[INFO] Hostname\t\t: " + getHostId());
    Serial.println("-->[INFO] Revision\t\t: " + gui.getFirmwareVersionCode());
    Serial.println("-->[INFO] Firmware\t\t: " + String(VERSION));
    Serial.println("-->[INFO] Flavor  \t\t: " + String(FLAVOR));
    Serial.println("-->[INFO] Target  \t\t: " + String(TARGET)); 
    logMemory("GPIO");  
    // Sensors library initialization
    Serial.println("-->[INFO] == Detecting Sensors ==");
    Serial.println("-->[INFO] Sensorslib version\t: " + sensors.getLibraryVersion());
    startingSensors();
    logMemory("SLIB");
    // Setting callback for remote commands via Bluetooth config
    setRemoteConfigCallbacks(new MyRemoteConfigCallBacks());
    // init watchdog timer for reboot in any loop blocker
    wd.init();
    // WiFi and cloud communication
    logMemory("WDOG");
    gui.welcomeAddMessage("Connecting..");
    wifiInit();
    logMemory("WIFI");
    Serial.printf("-->[INFO] InfluxDb cloud \t: %s\r\n", isIfxEnable()  ? "enabled" : "disabled");
    Serial.printf("-->[INFO] WiFi current config\t: %s\r\n", isWifiEnable() ? "enabled" : "disabled");

    String sname = !(cfg.getString("geo", "")).isEmpty() ? getStationName() : "not configured yet\t:(";
    Serial.printf("-->[INFO] CanAirIO station name\t: %s\r\n", sname.c_str());
    gui.welcomeAddMessage("WiFi: "+String(isWifiEnable() ? "On" : "Off"));
    gui.welcomeAddMessage("Influx: "+String(isIfxEnable() ? "On" : "Off"));

#ifndef DISABLE_BLE
    // Bluetooth low energy init (GATT server for device config)
    bleServerInit();
    logMemory("BLE ");
    gui.welcomeAddMessage("Bluetooth ready.");
#endif

    // wifi status 
    if (isWifiEnable() && WiFi.isConnected()) {
        Serial.printf("-->[INFO] Wifi connected to\t: %s\r\n", WiFi.SSID().c_str());
        gui.welcomeAddMessage("WiFi:" + cfg.getString(CONFKEYS::KSSID, ""));
    }
    else {
        Serial.printf("-->[INFO] Wifi connected to\t: disabled\r\n");
        gui.welcomeAddMessage("WiFi: disabled.");
    }

    // sensor sample time and publish time (2x)
    gui.welcomeAddMessage("stime: "+String(stime)+ " sec.");
    gui.welcomeAddMessage(getDeviceId());   // mac address
    gui.welcomeAddMessage("Watchdog:"+String(WATCHDOG_TIME)); 
    gui.welcomeAddMessage("==SETUP READY==");
    delay(600);
    gui.showMain();
    gui.loop();
    refreshGUIData(false);
    logMemory("GLIB");
    Serial.printf("-->[INFO] sensors units count\t: %d\r\n", sensors.getUnitsRegisteredCount());
    Serial.printf("-->[INFO] show unit selected \t: %s\r\n",sensors.getUnitName(selectUnit).c_str()); 
    // testing workaround on init config.
    cfg.saveString("kdevid",getDeviceId());
    // enabling CLI interface
    logMemoryObjects();

    #ifdef LORADEVKIT
    LoRaWANSetup();
    logMemory("LORAWAN");    
    #endif
    initCLI();
}

void loop() {
  sensors.loop(); // read sensor data and showed it
  otaLoop();      // check for firmware updates
  snifferLoop();  // pax counter calc (only when WiFi is Off)
  wifiLoop();     // check wifi and reconnect it
  wd.loop();      // watchdog for check loop blockers
#ifndef DISABLE_BLE
  bleLoop();      // notify data to connected devices
  gui.setGUIStatusFlags(WiFi.isConnected(), true, bleIsConnected());
#endif
  gui.setGUIStatusFlags(WiFi.isConnected(), true, false);
  gui.loop();     // Only for OLED
#ifndef DISABLE_BATT
  battery.loop();  // refresh battery level and voltage
#endif
#ifdef LORADEVKIT
  os_runloop_once();
#endif
  powerLoop();    // check power status and manage power saving
}
