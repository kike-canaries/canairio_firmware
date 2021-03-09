#include <OTAHandler.h>

esp32FOTA esp32FOTA("ttgo-t7", SRC_REV);

OTAHandler::OTAHandler(){
    m_pOTAHandlerCallbacks = nullptr;
}

void OTAHandler::setup(const char* ESP_ID, const char* ESP_PASS) {
    _ESP_ID = ESP_ID;
    _ESP_PASS = ESP_PASS;
    _baud = 1500000;
    ArduinoOTA.setHostname(_ESP_ID);  
    ArduinoOTA.setPassword(_ESP_PASS);

    ArduinoOTA
        .onStart([]() {
            if(ota.getInstance()->m_pOTAHandlerCallbacks!=nullptr)
                ota.getInstance()->m_pOTAHandlerCallbacks->onStart();
        })
        .onEnd([]() {
            Serial.println("\n-->[OTA] success!");
            if(ota.getInstance()->m_pOTAHandlerCallbacks!=nullptr)
                ota.getInstance()->m_pOTAHandlerCallbacks->onEnd();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("-->[OTA] Progress: %u%%\r", (progress / (total / 100)));            
            if(ota.getInstance()->m_pOTAHandlerCallbacks!=nullptr)
                ota.getInstance()->m_pOTAHandlerCallbacks->onProgress(progress,total);
        })
        .onError([](ota_error_t error) {
            Serial.printf("-->[E][OTA] Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)         Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)     Serial.println("End Failed");
            if(ota.getInstance()->m_pOTAHandlerCallbacks!=nullptr)
                ota.getInstance()->m_pOTAHandlerCallbacks->onError();
        });

    ArduinoOTA.begin();
    
    // Remote OTA config
    // TODO: pass host and target via bluetooth
    esp32FOTA.checkURL = "http://influxdb.canair.io:8080/releases/firmware.json";
    
    Serial.println("-->[INFO] ready for OTA update.");
}

void OTAHandler::checkRemoteOTA() {
    bool updatedNeeded = esp32FOTA.execHTTPcheck();
    if (updatedNeeded) {
        Serial.println("-->[FOTA] starting..");
        esp32FOTA.execOTA();
    } else
        Serial.println("-->[FOTA] not need update");
}

void OTAHandler::remoteOTAcheckloop() {
    if ((millis() - FOTA_CHECK_INTERVAL*1000) > _lastOTACheck) {
        _lastOTACheck = millis();
        checkRemoteOTA();
    }
}

void OTAHandler::loop() {
    ArduinoOTA.handle();
    remoteOTAcheckloop();
}

void OTAHandler::setBaud(int baud) {
    _baud = baud;
}

void OTAHandler::setCallbacks(OTAHandlerCallbacks* pCallbacks) {
	m_pOTAHandlerCallbacks = pCallbacks;
}

OTAHandler* OTAHandler::getInstance() {
	return this;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OTAHANDLER)
OTAHandler ota;
#endif

