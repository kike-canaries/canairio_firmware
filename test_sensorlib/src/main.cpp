/**
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018 - 2020
 * @brief Particle meter sensor on ESP32 with bluetooth GATT notify server
 * @license GPL3
 */

#include <Arduino.h>

#include <Sensors.hpp>

void displayGUI() {
    static uint_fast64_t timeStampGUI = 0;   // timestamp for GUI refresh
    if ((millis() - timeStampGUI > 1000)) {  // it should be minor than sensor loop
        timeStampGUI = millis();
        Serial.println("-->[MAIN] PM2.5 value: "+sensors.getPM25());
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
    Serial.println("\n== Sensor test setup ==\n");

    sensors.setOnDataCallBack(&onSensorDataOk);  // all data read callback
    sensors.setSampleTime(5);            // config sensors sample time
    sensors.init(sensors.Sensirion);             // start all sensors
    // if(sensors.isPmSensorConfigured())
        // gui.welcomeRepeatMessage(sensors.getPmDeviceSelected());
    // else 
        // gui.welcomeRepeatMessage("Detection !FAIL!");
    delay(500);
}

void loop() {
    sensors.loop();  // read sensor data and showed it
}