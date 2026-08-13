#ifndef DISABLE_BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <bluetooth.hpp>
#include <sniffer.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <string>

BLEServer* pServer = NULL;
BLECharacteristic* pCharactData = NULL;
BLECharacteristic* pCharactConfig = NULL;
BLECharacteristic* pCharactStatus = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

/*************************************************************************
*   T H R E A D   S A F E T Y   N O T E S
*
*  All BLECharacteristicCallbacks / BLEServerCallbacks run on the Bluedroid
*  BTC task, NOT on the Arduino loop task. On the Arduino core that task has
*  only CONFIG_BT_BTC_TASK_STACK_SIZE = 3072 bytes of stack.
*
*  Two consequences, both of them sources of crashes in the past:
*
*  1) Heavy work (ArduinoJson, NVS/Preferences, GUI drawing, verbose logging)
*     must NOT run inside the callbacks: it overflows the BTC stack, which
*     shows up as an unrelated assert (xQueueGenericSend, queue.c) instead of
*     a clean stack overflow message. Callbacks below only copy the payload
*     and raise a flag; bleLoop() does the real work on the loop task.
*
*  2) The characteristic value is touched from both tasks, so every
*     setValue()/getValue() goes through the bleMtx mutex.
*
*  Related issues: #302 #303 #337
*************************************************************************/

static SemaphoreHandle_t bleMtx = NULL;        // guards characteristic values
static SemaphoreHandle_t blePendingMtx = NULL; // guards the deferred buffers

// deferred work, filled from the BTC task, consumed by the Arduino loop task
static volatile bool pendingConfigRead = false;
static volatile bool pendingConfigWrite = false;
static volatile bool pendingStatusWrite = false;
static std::string pendingConfigPayload;
static std::string pendingStatusPayload;

static void safeSetValue(BLECharacteristic* pCharact, const String& value) {
    if (bleMtx != NULL && xSemaphoreTake(bleMtx, pdMS_TO_TICKS(50)) == pdTRUE) {
        pCharact->setValue(value.c_str());
        xSemaphoreGive(bleMtx);
    } else {
        log_w("[BTLE] safeSetValue: mutex timeout, value not updated");
    }
}

static std::string safeGetValue(BLECharacteristic* pCharact) {
    std::string value;
    if (bleMtx != NULL && xSemaphoreTake(bleMtx, pdMS_TO_TICKS(50)) == pdTRUE) {
        value = pCharact->getValue();
        xSemaphoreGive(bleMtx);
    }
    return value;
}

/*************************************************************************
*   B L U E T O O T H   P A Y L O A D
*************************************************************************/

String getNotificationData() {
    JsonDocument doc;   // notification capacity is reduced, only main value
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
    JsonDocument doc;
    
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
    #ifndef DISABLE_BATT
    doc["bat"] = battery.getCharge();
    doc["vol"] = battery.getVoltage();
    #endif
    doc["PAX"] = getPaxCount();
    doc["dsl"] = sensors.getSensorName((SENSORS) sensors.getUARTDeviceTypeSelected());
    String json;
    serializeJson(doc, json);
    return json;
}

/*************************************************************************
*   B L U E T O O T H   M E T H O D S   (loop task only)
*************************************************************************/

void bleServerDataRefresh(){
    safeSetValue(pCharactData, getSensorData());
}

// WARNING: builds the config JSON (ArduinoJson + ~25 NVS reads). Call it only
// from the Arduino loop task, never from a BLE callback.
void bleServerConfigRefresh(){
    setWifiConnected(WiFi.isConnected());  // for notify on each write
    safeSetValue(pCharactConfig, getCurrentConfig());
}

/*************************************************************************
*   B L E   C A L L B A C K S   (BTC task: keep them minimal)
*************************************************************************/

// Config BLE callbacks
class MyConfigCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = safeGetValue(pCharacteristic);
        if (value.length() == 0) return;
        if (blePendingMtx != NULL && xSemaphoreTake(blePendingMtx, pdMS_TO_TICKS(50)) == pdTRUE) {
            pendingConfigPayload = value;   // deferred: save() runs on the loop task
            pendingConfigWrite = true;
            xSemaphoreGive(blePendingMtx);
        }
    };

    void onRead(BLECharacteristic* pCharacteristic) {
        // deferred: the value was already refreshed by bleLoop(), just ask for
        // a new refresh after answering this read.
        pendingConfigRead = true;
    }
};

// Status BLE callbacks
class MyStatusCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = safeGetValue(pCharacteristic);
        if (value.length() == 0) return;
        if (blePendingMtx != NULL && xSemaphoreTake(blePendingMtx, pdMS_TO_TICKS(50)) == pdTRUE) {
            pendingStatusPayload = value;   // deferred: GUI update on the loop task
            pendingStatusWrite = true;
            xSemaphoreGive(blePendingMtx);
        }
    }
};

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        pendingConfigRead = true;  // refresh config for the incoming client
        Serial.println("-->[BTLE] device client is connected.");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("-->[BTLE] device client is disconnected.");
    };
};  // BLEServerCallbacks

/*************************************************************************
*   D E F E R R E D   W O R K   (loop task)
*************************************************************************/

static void bleProcessPendingWork() {
    if (pendingConfigWrite) {
        std::string payload;
        if (blePendingMtx != NULL && xSemaphoreTake(blePendingMtx, pdMS_TO_TICKS(50)) == pdTRUE) {
            payload = pendingConfigPayload;
            pendingConfigWrite = false;
            xSemaphoreGive(blePendingMtx);
        }
        if (payload.length() > 0) {
            if (save(payload.c_str())) {
                gui.displayPreferenceSaveIcon();
                gui.setWifiMode(isWifiEnable());

                if (sensors.sample_time != stime) {
                    sensors.setSampleTime(stime);
                    gui.setSampleTime(stime);
                }
                if (sensors.toffset != toffset) sensors.setTempOffset(toffset);
                if (sensors.devmode != devmode) sensors.setDebugMode(devmode);
            } else {
                log_w("[W][BTLE][CONFIG] save return false with %s", payload.c_str());
            }
            pendingConfigRead = true;  // publish the updated config
        }
    }

    if (pendingStatusWrite) {
        std::string payload;
        if (blePendingMtx != NULL && xSemaphoreTake(blePendingMtx, pdMS_TO_TICKS(50)) == pdTRUE) {
            payload = pendingStatusPayload;
            pendingStatusWrite = false;
            xSemaphoreGive(blePendingMtx);
        }
        if (payload.length() > 0) {
            if (getTrackStatusValues(payload.c_str())) {
                log_v("[BTLE][STATUS] %s", payload.c_str());
                gui.setTrackValues(track.spd, track.kms);
                gui.setTrackTime(track.hrs, track.min, track.seg);
            } else {
                Serial.println("[E][BTLE][STATUS] write error!");
            }
        }
    }

    if (pendingConfigRead) {
        pendingConfigRead = false;
        bleServerConfigRefresh();
    }
}

/*************************************************************************
*   S E R V E R   I N I T   /   L O O P
*************************************************************************/

void bleServerInit() {
    // Create the mutexes before any setValue() call
    if (bleMtx == NULL) bleMtx = xSemaphoreCreateMutex();
    if (blePendingMtx == NULL) blePendingMtx = xSemaphoreCreateMutex();
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
    // deferred work coming from the BLE callbacks (runs on this task)
    bleProcessPendingWork();
    // notify changed value
    if (deviceConnected && (millis() - bleTimeStamp > stime * 1000)) {  // each 5 secs
        log_i("[BTLE] sending notification..");
        bleTimeStamp = millis();
        String payload = getNotificationData();  // small payload for notification
        log_d("[BTLE] %s", payload.c_str());
        if (bleMtx != NULL && xSemaphoreTake(bleMtx, pdMS_TO_TICKS(50)) == pdTRUE) {
            pCharactData->setValue(payload.c_str());
            pCharactData->notify();
            xSemaphoreGive(bleMtx);
        }
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
#endif