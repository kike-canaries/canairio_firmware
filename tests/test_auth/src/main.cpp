
/**
 * Authentication tests
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018 - 2019
 * @brief CanAirIO API tests
 * @license GPL3
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Wire.h>

#include <CanAirIoApi.hpp>
#include <ConfigApp.hpp>
#include <GUIUtils.hpp>

unsigned int apm25 = 0;  // last PM2.5 average
unsigned int apm10 = 0;  // last PM10 average
int stime = 5;           // sample time (send data each 5 sec)
double lat, lon;         // Coordinates
float alt, spd;          // Altitude and speed

// WiFi fields
#define WIFI_RETRY_CONNECTION 20
bool dataSendToggle;
bool wifiOn;

// CanAirIO API fields
CanAirIoApi api(true);

// GUI fields
#define LED 2

/******************************************************************************
*   W I F I   M E T H O D S
******************************************************************************/

bool wifiCheck() {
    wifiOn = WiFi.isConnected();
    return wifiOn;
}

void wifiConnect(const char* ssid, const char* pass) {
    Serial.print("-->[WIFI] Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, pass);
    int wifi_retry = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_retry++ < WIFI_RETRY_CONNECTION) {
        Serial.print(".");
        delay(250);
    }
    if (wifiCheck()) {
        Serial.println("done\n-->[WIFI] connected!");
    } else {
        Serial.println("fail!\n-->[E][WIFI] disconnected!");
    }
}

void wifiInit() {
    if (cfg.wifiEnable && cfg.ssid.length() > 0 && cfg.pass.length() > 0) {
        wifiConnect(cfg.ssid.c_str(), cfg.pass.c_str());
    }
}

void wifiStop() {
    if (wifiOn) {
        Serial.println("-->[WIFI] Disconnecting..");
        WiFi.disconnect(true);
        delay(100);
        wifiCheck();
    }
}

void wifiRestart() {
    wifiStop();
    wifiInit();
}

void wifiLoop() {
    static uint_least64_t wifiTimeStamp = 0;
    if (millis() - wifiTimeStamp > 5000 && cfg.wifiEnable && cfg.ssid.length() > 0 && !wifiCheck()) {
        wifiTimeStamp = millis();
        wifiConnect(cfg.ssid.c_str(), cfg.pass.c_str());
    }
}

/******************************************************************************
*   C A N A I R I O  P U B L I S H   M E T H O D S
******************************************************************************/
void apiInit() {
    if (WiFi.isConnected()) {
        Serial.println("-->[API] Connecting..");
        // stationId and deviceId, optional endpoint, host and port
        api.dev = true;
        if (cfg.apiuri.equals("") && cfg.apisrv.equals(""))
            api.configure(cfg.dname.c_str(), cfg.deviceId);
        else
            api.configure(cfg.dname.c_str(), cfg.deviceId, cfg.apiuri.c_str(), cfg.apisrv.c_str(), cfg.apiprt);
        api.authorize(cfg.apiusr.c_str(), cfg.apipss.c_str());
        // api.authorize("canairio", "canairio_password");
        delay(1000);
    }
}

void apiLoop() {
    if (wifiOn) {
        Serial.print("-->[API] write..");
        if (api.write(0, apm25, apm10, 5.23, 3.50, 1.23, 12.35, 23.20, 30.10, 10)) {
            Serial.println("done!");
            gui.welcomeAddMessage("test: pass!");
            while (1)
                ;
        } else {
            Serial.println("failed!");
            gui.welcomeAddMessage("test:fail! " + String(api.getResponse()));
            while (1)
                ;
        }
    }
}

/******************************************************************************
*  M A I N
******************************************************************************/

void setup() {
    Serial.begin(115200);
    Serial.println("\n== INIT SETUP ==\n");
    gui.displayInit();
    gui.showWelcome();
    cfg.init("canairio");
    Serial.println("-->[INFO] ESP32MAC: " + String(cfg.deviceId));
    if (cfg.ssid.length() > 0)
        gui.welcomeAddMessage("WiFi:" + cfg.ssid);
    else
        gui.welcomeAddMessage("WiFi radio test..");
    wifiInit();
    gui.welcomeAddMessage("CanAirIO API..");
    apiInit();
    pinMode(LED, OUTPUT);
    gui.welcomeAddMessage("==SETUP READY==");
    delay(500);
    gui.welcomeAddMessage("testing API..");
}

void loop() {
    delay(5000);
    apiLoop();
}