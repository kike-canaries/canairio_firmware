#include <OTAHandler.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "esp_system.h"

OTAHandler::OTAHandler(){
}

void OTAHandler::setup(const char* ESP_ID, const char* ESP_PASS) {
    _ESP_ID = ESP_ID;
    _ESP_PASS = ESP_PASS;
    _baud = 460800;
    // everloop green
    Serial.println("-->[OTA] Booting..");
    //everloop blue
    ArduinoOTA.setHostname(_ESP_ID);  
    ArduinoOTA.setPassword(_ESP_PASS);
    ArduinoOTA
        .onStart([]() {
        })
        .onEnd([]() {
        Serial.println("\n-->[OTA] End");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("-->[OTA] Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
        Serial.printf("-->[E][OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

    ArduinoOTA.begin();
    Serial.println("-->[OTA] Ready");
}

void OTAHandler::loop() {
    ArduinoOTA.handle();
}

void OTAHandler::setBaud(int baud) {
    _baud = baud;
}
