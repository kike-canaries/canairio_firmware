#include <cloud_anaire.hpp>
#include <wifi.hpp>

/******************************************************************************
*  A N A I R E   M Q T T   M E T H O D S
******************************************************************************/

WiFiClient net;
MQTTClient client;

void anairePublish() {
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
        client.publish(ANAIRE_TOPIC, MQTT_message);
    }
}

static uint_fast64_t mqttDelayedStamp = 0;

void anaireConnect() {
    if (!(cfg.isWifiEnable() && WiFi.isConnected())) return;

    if (millis() - mqttDelayedStamp > MQTT_DELAYED_TIME * 1000) {
        Serial.printf("-->[MQTT] Anaire connecting to %s..", ANAIRE_HOST);
        int mqtt_try = 0;
        while (mqtt_try++ < MQTT_RETRY_CONNECTION && !client.connect(cfg.getStationName().c_str())) {
            Serial.print(".");
            delay(100);
        }
        if (mqtt_try >= MQTT_RETRY_CONNECTION && !client.connected()) {
            mqttDelayedStamp = millis();
            Serial.println("connection failed!");
            return;
        }
        mqttDelayedStamp = millis();
        Serial.println("connected!");
        client.subscribe(ANAIRE_TOPIC);
    }
}

void anaireInit() { 
    Serial.println("-->[MQTT] Anaire init");
    client.begin(ANAIRE_HOST, ANAIRE_PORT, net);
    mqttDelayedStamp = millis() - MQTT_DELAYED_TIME * 1000;
    anaireConnect();
}

void anaireLoop () {
    if(!WiFi.isConnected()) return; 
    client.loop();
    delay(10);
    if (!client.connected()) anaireConnect();
    anairePublish();
}