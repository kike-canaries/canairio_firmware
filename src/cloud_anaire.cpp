#include <cloud_anaire.hpp>
#include <wifi.hpp>
#include <Batterylib.hpp>

/******************************************************************************
*  A N A I R E   M Q T T   M E T H O D S
******************************************************************************/

WiFiClient netAnaire;
MQTTClient client(MQTT_BUFFER_SIZE);

void anairePublish() {
    static uint_fast64_t mqttTimeStamp = 0;
    uint32_t ptime = cfg.stime;
    if (ptime<MIN_PUBLISH_INTERVAL) ptime = MIN_PUBLISH_INTERVAL-1; // publish before to the last cloud
    if(!cfg.solarmode && cfg.deepSleep > 0) ptime = cfg.deepSleep;
    if (millis() - mqttTimeStamp > ptime * 1000) {
        mqttTimeStamp = millis();

        float humi = sensors.getHumidity();
        if (humi == 0.0) humi = sensors.getCO2humi();
        float temp = sensors.getTemperature();
        if (temp == 0.0) temp = sensors.getCO2temp();

        StaticJsonDocument<MQTT_BUFFER_SIZE> doc;
        char buffer[MQTT_BUFFER_SIZE];

        doc["id"] = cfg.getStationName();
        doc["ver"] = cfg.getVersion();
        doc["CO2"] = String(sensors.getCO2());
        doc["humidity"] = String(humi);
        doc["temperature"] = String(temp);
        doc["pressure"] = String(sensors.getPressure());
        doc["altitude"] = String(sensors.getAltitude());
        doc["gas"] = String(sensors.getGas());
        doc["pm1"] = sensors.getPM1();
        doc["pm25"] = sensors.getPM25();
        doc["pm10"] = sensors.getPM10();
        doc["cpm"] = String(sensors.getGeigerCPM());
        doc["usvh"] = String(sensors.getGeigerMicroSievertHour());
        doc["geo"] = cfg.geo;
        doc["battery"] = String(battery.getCharge());
        doc["VBat"] = String(battery.getVoltage());
        doc["co"] = String(sensors.getCO());
        doc["nh3"] = String(sensors.getNH3());
        doc["no2"] = String(sensors.getNO2());

        size_t n = serializeJson(doc, buffer);

        if (client.publish(ANAIRE_TOPIC, buffer, n)) {
            if (cfg.devmode) Serial.printf("-->[MQTT] Anaire published\t: payload size: %d\t:)\r\n", n);
        } else {
            if(client.lastError()!=0)
                Serial.printf("[E][MQTT] Anaire publish error \t: %d\r\n",client.lastError());
        }
    }
}

static uint_fast64_t mqttDelayedStamp = 0;

void anaireConnect() {
    if (!(cfg.isWifiEnable() && WiFi.isConnected())) return;

    if (millis() - mqttDelayedStamp > MQTT_DELAYED_TIME * 1000) {
        if (cfg.devmode) Serial.printf("-->[MQTT] %s\t: ", ANAIRE_HOST);
        int mqtt_try = 0;
        while (mqtt_try++ < MQTT_RETRY_CONNECTION && !client.connect(cfg.getStationName().c_str())) {
            delay(100);
        }
        if (mqtt_try >= MQTT_RETRY_CONNECTION && !client.connected()) {
            mqttDelayedStamp = millis();
            if (cfg.devmode) Serial.println("connection failed!");
            return;
        }
        mqttDelayedStamp = millis();
        if (cfg.devmode) Serial.println("connected!");
        client.subscribe(ANAIRE_TOPIC);
    }
}

bool anaireIsConnected() {
    return client.connected();
}

void anaireInit() { 
    client.begin(ANAIRE_HOST, ANAIRE_PORT, netAnaire);
    mqttDelayedStamp = millis() - MQTT_DELAYED_TIME * 1000;
    anaireConnect();
}

void anaireLoop () {
    if(!WiFi.isConnected()) return;
    if (!client.connected()) {
      anaireInit();
      delay(10);
    }
    client.loop();
    delay(10);
    anairePublish();
}