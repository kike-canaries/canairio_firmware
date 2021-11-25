#include <cloud_hass.hpp>
#include <wifi.hpp>

/******************************************************************************
*  H A S S   M Q T T   M E T H O D S
******************************************************************************/

WiFiClient netHass;
MQTTClient clientHass(MQTT_BUFFER_SIZE);

String getRootTopic() {
    return String(HPREFIX) + String(HCOMP) + getHostId();
}

String getHassTopic(const char topic[]) {
    return getRootTopic() + "/" + String(topic);
}

void publishSensorPayload() {
    char MQTT_message[MQTT_BUFFER_SIZE];

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
    if (clientHass.publish(getHassTopic(TOPIC_STATE).c_str(), MQTT_message)) return;
    Serial.printf("[E][MQTT] last error: %d\n",clientHass.lastError());
}

void publishDiscoveryConfigPayload() {
    StaticJsonDocument<MQTT_BUFFER_SIZE> doc;
    doc["name"] = "CanAirIO Device";
    doc["stat_t"] = getHassTopic(TOPIC_STATE);
    doc["uniq_id"] = getHostId()+"_temperature_sensor";
    doc["dev_cla"] = "temperature";
    doc["unit_of_meas"] = "°C";
    // doc["val_tpl"] = "{{ value_json.temperature }}";
    char MQTT_message[MQTT_BUFFER_SIZE];
    size_t n = serializeJson(doc, MQTT_message);
    if (clientHass.publish(getHassTopic(TOPIC_CONF).c_str(), MQTT_message, n)) return;
    Serial.printf("[E][MQTT] last error: %d\n",clientHass.lastError());
}

void hassDiscoverySubscribe() {
    clientHass.subscribe(getHassTopic(TOPIC_CONF).c_str());
}

void hassStateSubscribe() {
    clientHass.subscribe(getHassTopic(TOPIC_STATE).c_str());
}

void hassPublish() {
    static uint_fast64_t mqttTimeStamp = 0;
    if (millis() - mqttTimeStamp > cfg.stime * 1000) {
        mqttTimeStamp = millis();
        publishDiscoveryConfigPayload(); 
        publishSensorPayload();
    }
}

static uint_fast64_t mqttHassDelayedStamp = 0;

void hassConnect() {
    if (!(cfg.isWifiEnable() && WiFi.isConnected())) return;
    if (millis() - mqttHassDelayedStamp > MQTT_DELAYED_TIME * 1000) {
        Serial.printf("-->[MQTT] connecting to: %s..", HASS_HOST);
        int mqtt_try = 0;
        while (mqtt_try++ < MQTT_RETRY_CONNECTION && !clientHass.connect(cfg.getStationName().c_str())) {
            Serial.print(".");
            delay(100);
        }
        if (mqtt_try >= MQTT_RETRY_CONNECTION && !clientHass.connected()) {
            mqttHassDelayedStamp = millis();
            Serial.println("\tconnection failed!");
            return;
        }
        mqttHassDelayedStamp = millis();
        // hassStateSubscribe();
        Serial.println("\tconnected!");
    }
}

void hassInit() { 
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