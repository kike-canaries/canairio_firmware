#include <bluetooth.hpp>
#include "ConfigApp.hpp"

BLEServer* pServer = NULL;
BLECharacteristic* pCharactData = NULL;
BLECharacteristic* pCharactConfig = NULL;
BLECharacteristic* pCharactStatus = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

/*************************************************************************
*   B L U E T O O T H   P A Y L O A D
*************************************************************************/

String getNotificationData() {
    StaticJsonDocument<40> doc;   // notification capacity is reduced, only main value
    int deviceType = sensors.getUARTDeviceTypeSelected();
    if (deviceType <= 3) {
        doc["P25"] = sensors.getPM25();  
    } else {
        doc["CO2"] = sensors.getCO2();
    }
    String json;
    serializeJson(doc, json);
    return json;
}

String getSensorData() {
    StaticJsonDocument<512> doc;
    
    doc["P1"] = sensors.getPM1();
    doc["P25"] = sensors.getPM25();
    doc["P4"] = sensors.getPM4();
    doc["P10"] = sensors.getPM10();
    doc["CO2"] = sensors.getCO2();
    doc["CO2T"] = sensors.getCO2temp();
    doc["CO2H"] = sensors.getCO2humi();
    doc["tmp"] = sensors.getTemperature();
    doc["hum"] = sensors.getHumidity();
    doc["alt"] = sensors.getAltitude();
    doc["pre"] = sensors.getPressure();
    doc["nh3"] = sensors.getNH3();
    doc["co"] = sensors.getCO();
    doc["bat"] = battery.getCharge();
    doc["vol"] = battery.getVoltage();
    doc["PAX"] = getPaxCount();
    doc["dsl"] = sensors.getSensorName((SENSORS) sensors.getUARTDeviceTypeSelected());
    String json;
    serializeJson(doc, json);
    return json;
}

/*************************************************************************
*   B L U E T O O T H   M E T H O D S
*************************************************************************/

void bleServerDataRefresh(){
    pCharactData->setValue(getSensorData().c_str());
}

void bleServerConfigRefresh(){
    if (FAMILY == "ESP32-C3") return;
    setWifiConnected(WiFi.isConnected());  // for notify on each write
    pCharactConfig->setValue(getCurrentConfig().c_str());
}

// Config BLE callbacks
class MyConfigCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        if (FAMILY == "ESP32-C3") return;
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            if (save(value.c_str())) {
                reload();
                gui.displayPreferenceSaveIcon();
                
                gui.setWifiMode(isWifiEnable());
                if (!isWifiEnable()) wifiStop();
                
                if(sensors.sample_time != stime) {
                    sensors.setSampleTime(stime);
                    gui.setSampleTime(stime);
                }
                if(sensors.toffset != toffset) sensors.setTempOffset(toffset);
                if(sensors.devmode != devmode) sensors.setDebugMode(devmode);
            }
            else{
                Serial.println("[E][BTLE][CONFIG] saving error!");
            }
        }
    };

    void onRead(BLECharacteristic* pCharacteristic) {
        if (FAMILY == "ESP32-C3") return;
        bleServerConfigRefresh();
    }
};

// Status BLE callbacks
class MyStatusCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        if (FAMILY == "ESP32-C3") return;
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0 && getTrackStatusValues(value.c_str())) {
            log_v("[E][BTLE][STATUS] "+String(value.c_str()));
            gui.setTrackValues(track.spd,track.kms);
            gui.setTrackTime(track.hrs,track.min,track.seg);
        }
        else {
            Serial.println("[E][BTLE][STATUS] write error!");
        }
    }
};

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("-->[BTLE] device client is connected.");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("-->[BTLE] device client is disconnected.");
    };
};  // BLEServerCallbacks



void bleServerInit() {
    // Create the BLE Device
    BLEDevice::init("CanAirIO_ESP32");
    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    // Create the BLE Service
    BLEService* pService = pServer->createService(SERVICE_UUID);
    // Create a BLE Characteristic for PM 2.5
    pCharactData = pService->createCharacteristic(
        CHARAC_DATA_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    // Create a BLE Characteristic for Sensor mode: STATIC/MOVIL
    pCharactConfig = pService->createCharacteristic(
        CHARAC_CONFIG_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    // Create a BLE Characteristic for Sensor mode: STATIC/MOVIL
    pCharactStatus = pService->createCharacteristic(
        CHARAC_STATUS_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    // Config callback
    pCharactConfig->setCallbacks(new MyConfigCallbacks());
    // Status callback
    pCharactStatus->setCallbacks(new MyStatusCallbacks());
    // Set callback data:
    bleServerConfigRefresh();
    bleServerDataRefresh();
    // Create a Data Descriptor (for notifications)
    pCharactData->addDescriptor(new BLE2902());
    // Start the service
    pService->start();
    // Start advertising
    pServer->getAdvertising()->start();
    Serial.println("-->[BTLE] Bluetooth GATT \t: ready for config client!");
}

void bleLoop() {
    static uint_fast64_t bleTimeStamp = 0;
    // notify changed value
    if (deviceConnected && (millis() - bleTimeStamp > stime * 1000)) {  // each 5 secs
        log_i("[BTLE] sending notification..");
        log_d("[BTLE] %s",getNotificationData().c_str());
        bleTimeStamp = millis();
        pCharactData->setValue(getNotificationData().c_str());  // small payload for notification
        pCharactData->notify();
        bleServerDataRefresh();
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(250);                   // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising();  // restart advertising
        Serial.println("-->[BTLE] start advertising..");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}

bool bleIsConnected(){
    return deviceConnected;
}
