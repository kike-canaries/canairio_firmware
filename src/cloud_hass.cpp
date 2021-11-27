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

String getStateTopic(String dclass) {
    return getRootTopic()+"/"+dclass+"/"+String(TOPIC_STATE);
}

String getConfTopic(String name) {
    return getRootTopic()+"/"+name+"/"+String(TOPIC_CONF);
}

void hassPubSensorPayload(String dclass) {

    float humi = sensors.getHumidity();
    if (humi == 0.0) humi = sensors.getCO2humi();
    float temp = sensors.getTemperature();
    if (temp == 0.0) temp = sensors.getCO2temp();

    String output = "";

    if (dclass.equals("carbon_dioxide")) output = String(sensors.getCO2());
    else if (dclass.equals("humidity")) output = String(humi);
    else if (dclass.equals("temperature")) output = String(temp);
    else if (dclass.equals("pressure")) output = String(sensors.getPressure());
    else if (dclass.equals("altitude")) output = String(sensors.getAltitude());
    else if (dclass.equals("gas")) output = String(sensors.getGas());
    else if (dclass.equals("pm1")) output = String(sensors.getPM1());
    else if (dclass.equals("pm25")) output = String(sensors.getPM25());
    else if (dclass.equals("pm4")) output = String(sensors.getPM4());
    else if (dclass.equals("pm10")) output = String(sensors.getPM10());
 
    if (clientHass.publish(getStateTopic(dclass).c_str(), output)) return;
    Serial.printf("[E][MQTT] last error: %d\n",clientHass.lastError());
}

void publishDiscoveryPayload(String name, String dclass, String unit) {
    StaticJsonDocument<MQTT_BUFFER_SIZE> doc;
    doc["name"] = getHostId()+name;
    JsonObject device = doc.createNestedObject("device");
    device["manufacturer"] = "CanAirIO";
    device["model"] = ""+String(FLAVOR);
    device["name"] = getHostId()+name;
    device["sw_version"] = "v"+String(VERSION)+" rev"+String(REVISION);
    JsonArray identifiers = device.createNestedArray("identifiers");
    identifiers.add(getHostId()+name);
    doc["state_topic"] = getStateTopic(dclass);
    // JsonArray attributes = doc.createNestedArray("json_attributes");
    // attributes.add(dclass);
    // doc["json_attributes_topic"] = getStateTopic();
    doc["uniq_id"] = getHostId()+name;
    doc["dev_cla"] = dclass;
    doc["unit_of_meas"] = unit;
    // doc["frc_upd"] = true;
    // doc["value_template"] = tpl;
    char MQTT_message[MQTT_BUFFER_SIZE];
    size_t n = serializeJson(doc, MQTT_message);
     
    if (clientHass.publish(getConfTopic(dclass).c_str(), MQTT_message, n)) return;
    Serial.printf("[E][MQTT] last error: %d\n",clientHass.lastError());

}

void hassPubDiscoveryTemp() {
    publishDiscoveryPayload("temperature", "temperature", "°C");
}

void hassPubDiscoveryHumi() {
    publishDiscoveryPayload("humidity", "humidity", "%");
}

void hassPubDiscoveryCO2() {
    publishDiscoveryPayload("carbon_dioxide", "carbon_dioxide", "ppm");
}

void hassPubDiscoveryPM25() {
    publishDiscoveryPayload("pm25", "pm25", "µg/m³");
}

void hassPubAllSensors() {
    hassPubSensorPayload("temperature");
    hassPubSensorPayload("humidity");
    hassPubSensorPayload("carbon_dioxide");
    hassPubSensorPayload("pm25");
}

void hassRegisterSensors() {
    hassPubDiscoveryTemp();
    hassPubDiscoveryHumi();
    hassPubDiscoveryCO2();
    hassPubDiscoveryPM25();
}

void hassPublish() {
    static uint_fast64_t mqttTimeStamp = 0;
    if (millis() - mqttTimeStamp > cfg.stime * 1000) {
        mqttTimeStamp = millis(); 
        hassPubAllSensors();
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
        hassRegisterSensors();

        Serial.println("\tconnected!");
    }
}

bool isHassEnable() {
    return !cfg.hassip.isEmpty();
}

void hassInit() { 
    if (!isHassEnable()) return;
    clientHass.begin(cfg.hassip.c_str(), cfg.hasspt, netHass);
    mqttHassDelayedStamp = millis() - MQTT_DELAYED_TIME * 1000;
    hassConnect();
}

void hassLoop () {
    if (!isHassEnable()) return;
    if(!WiFi.isConnected()) return; 
    clientHass.loop();
    delay(10);
    if (!clientHass.connected()) hassConnect();
    hassPublish();
}