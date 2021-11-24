#include <cloud_hass.hpp>
#include <wifi.hpp>

/******************************************************************************
*  H A S S   M Q T T   M E T H O D S
******************************************************************************/

WiFiClient netHass;
MQTTClient clientHass;

void hassPublish() {
    static uint_fast64_t mqttTimeStamp = 0;
    if (millis() - mqttTimeStamp > cfg.stime * 1000) {
        mqttTimeStamp = millis();
        char MQTT_message[256];

        int deviceType = sensors.getPmDeviceTypeSelected();

        float humi = sensors.getHumidity();
        if (humi == 0.0) humi = sensors.getCO2humi();
        float temp = sensors.getTemperature();
        if (temp == 0.0) temp = sensors.getCO2temp();

        if (deviceType <= 3) {
            sprintf(MQTT_message, "{id: %s, pm1: %d, pm25: %d, pm10: %d, tmp: %f, hum: %f, geo: %s}",
                    cfg.getStationName().c_str(),
                    sensors.getPM1(),
                    sensors.getPM25(),
                    sensors.getPM10(),
                    temp,
                    humi,
                    cfg.geo.c_str());
        } else {
            sprintf(MQTT_message, "{id: %s,CO2: %d, humidity: %f, temperature: %f,VBat: %f}",
                    cfg.getStationName().c_str(),
                    sensors.getCO2(),
                    humi,
                    temp,
                    0.0);
        }
        clientHass.publish(HASS_TOPIC, MQTT_message);
    }
}

static uint_fast64_t mqttHassDelayedStamp = 0;

void hassConnect() {
    if (!(cfg.isWifiEnable() && WiFi.isConnected())) return;
    if (millis() - mqttHassDelayedStamp > MQTT_DELAYED_TIME * 1000) {
        Serial.printf("-->[MQTT] Hass connecting to %s..", HASS_HOST);
        int mqtt_try = 0;
        while (mqtt_try++ < MQTT_RETRY_CONNECTION && !clientHass.connect(cfg.getStationName().c_str())) {
            Serial.print(".");
            delay(100);
        }
        if (mqtt_try >= MQTT_RETRY_CONNECTION && !clientHass.connected()) {
            mqttHassDelayedStamp = millis();
            Serial.println("connection failed!");
            return;
        }
        mqttHassDelayedStamp = millis();
        Serial.println("connected!");
        clientHass.subscribe(HASS_TOPIC);
    }
}

void hassInit() { 
    Serial.println("-->[MQTT] Hass init");
    clientHass.begin(HASS_HOST, HASS_PORT, netHass);
    mqttHassDelayedStamp = millis() - MQTT_DELAYED_TIME * 1000;
    hassConnect();
}

void hassLoop () {
    if(!WiFi.isConnected()) return; 
    clientHass.loop();
    delay(10);
    if (!clientHass.connected()) hassConnect();
    hassPublish();
}