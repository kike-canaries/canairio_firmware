#include <cloud_hass.hpp>
#include <wifi.hpp>

/******************************************************************************
*  H A S S   M Q T T   M E T H O D S
******************************************************************************/

WiFiClient netHass;
MQTTClient clientHass(MQTT_BUFFER_SIZE);

bool hassInited = false;
bool hassConfigured = false;
bool hassSubscribed = false;

String getRootTopic() {
    return String(HPREFIX) + String(HCOMP) + getHostId();
}

String getStateTopic() {
    return getRootTopic()+"/"+String(TOPIC_STATE);
}

String getConfTopic(String name) {
    return getRootTopic()+"/"+name+"/"+String(TOPIC_CONF);
}

String getServerStatusTopic() {
    return String(HPREFIX) + String(TOPIC_STATUS);
}

void hassPubSensorPayload() {

    float humi = sensors.getHumidity();
    if (humi == 0.0) humi = sensors.getCO2humi();
    float temp = sensors.getTemperature();
    if (temp == 0.0) temp = sensors.getCO2temp();
    
    StaticJsonDocument<MQTT_BUFFER_SIZE> doc;
    char buffer[MQTT_BUFFER_SIZE];

    doc["carbon_dioxide"] = String(sensors.getCO2());
    doc["humidity"]    = String(humi);
    doc["temperature"] = String(temp);
    doc["pressure"] = String(sensors.getPressure());
    doc["altitude"] = String(sensors.getAltitude());
    doc["gas"]  = String(sensors.getGas());
    doc["pm1"]  = String(sensors.getPM1());
    doc["pm25"] = String(sensors.getPM25());
    doc["pm4"]  = String(sensors.getPM4());
    doc["pm10"] = String(sensors.getPM10());
    doc["battery"] = String(gui.getBatteryLevel());

    size_t n = serializeJson(doc, buffer);
 
    if (clientHass.publish(getStateTopic().c_str(), buffer, n)) return;
    Serial.printf("[E][MQTT] publish sensor state error: %d\n",clientHass.lastError());
}

bool publishDiscoveryPayload(String name, String dclass, String unit) {
    StaticJsonDocument<MQTT_BUFFER_SIZE> doc;
    doc["name"] = getHostId()+name; // name of the entity
    JsonObject device = doc.createNestedObject("device");
    device["manufacturer"] = "CanAirIO";
    device["model"] = ""+String(FLAVOR);
    device["name"] = getHostId();  // name of the device
    device["sw_version"] = "v"+String(VERSION)+" rev"+String(REVISION);
    JsonArray identifiers = device.createNestedArray("identifiers");
    identifiers.add(getHostId()); // name of the device
    doc["state_topic"] = getStateTopic();
    doc["state_class"] = "measurement",
    doc["uniq_id"] = getHostId()+name;
    doc["dev_cla"] = dclass;
    doc["unit_of_meas"] = unit;
    doc["value_template"] = "{{ value_json."+dclass+" }}";

    char MQTT_message[MQTT_BUFFER_SIZE];
    size_t n = serializeJson(doc, MQTT_message);
     
    if (clientHass.publish(getConfTopic(dclass).c_str(), MQTT_message, n)) return true;
    Serial.printf("[E][MQTT] publish %s config error: %d\n", dclass.c_str(), clientHass.lastError());
    return false;
}

void hassPubAllSensors() {
    hassPubSensorPayload();
    if(cfg.devmode) Serial.println ("-->[MQTT] Hass entities payload published.");
}

bool hassRegisterSensors() {
    hassConfigured = publishDiscoveryPayload("temperature", "temperature", "°C");
    hassConfigured = publishDiscoveryPayload("humidity", "humidity", "%");
    hassConfigured = publishDiscoveryPayload("carbon_dioxide", "carbon_dioxide", "ppm");
    hassConfigured = publishDiscoveryPayload("pm25", "pm25", "µg/m³");
    hassConfigured = publishDiscoveryPayload("gas", "gas", "m³");
    hassConfigured = publishDiscoveryPayload("pressure", "pressure", "hPa");
    hassConfigured = publishDiscoveryPayload("battery", "battery", "%");

    if (hassConfigured) Serial.printf("-->[MQTT] Hass device %s and entities registered.\n",getHostId().c_str());
    else Serial.printf("[E][MQTT] Hass device %s not configured yet...\n",getHostId().c_str());

    return hassConfigured;
}

void messageReceived(String &topic, String &payload) {
  Serial.println("-->[MQTT] incoming msg: " + topic + " - " + payload); 
  if (payload.equals("online")) hassRegisterSensors();
}

bool hassStatusSubscription() {
    if (clientHass.subscribe(getServerStatusTopic().c_str())) return true;
    Serial.printf("[E][MQTT] status subscription error: %d\n",clientHass.lastError());
    return false;
}

void hassPublish() {
    static uint_fast64_t mqttTimeStamp = 0;
    if (millis() - mqttTimeStamp > cfg.stime * 1000 * 2) {
        mqttTimeStamp = millis(); 
        if (!hassConfigured) hassRegisterSensors();
        if (!hassSubscribed) hassStatusSubscription();
        hassPubAllSensors();
    }
}

bool hassAuth() {
    return clientHass.connect(cfg.getStationName().c_str(), cfg.hassusr.c_str(), cfg.hasspsw.c_str());
}

static uint_fast64_t mqttHassDelayedStamp = 0;

void hassConnect() {
    if (!(cfg.isWifiEnable() && WiFi.isConnected())) return;
    if (millis() - mqttHassDelayedStamp > MQTT_DELAYED_TIME * 1000) {
        if (cfg.devmode) Serial.printf("-->[MQTT] Hass auth: %s@%s\n",cfg.hassusr.c_str(),cfg.hasspsw.c_str());
        Serial.printf("-->[MQTT] connecting to: %s:%i..", cfg.hassip.c_str(),cfg.hasspt);
        int mqtt_try = 0;
        while (mqtt_try++ < MQTT_RETRY_CONNECTION && !hassAuth()) {
            Serial.print(".");
            delay(100);
        }
        if (mqtt_try >= MQTT_RETRY_CONNECTION && !clientHass.connected()) {
            mqttHassDelayedStamp = millis();
            Serial.println("\tconnection failed!");
            return;
        }
        Serial.println("\tconnected!");
        mqttHassDelayedStamp = millis();
    }
}

bool isHassEnable() {
    if (cfg.hassip.isEmpty()) {
        hassInited = false;
        hassConfigured = false;
        hassSubscribed = false;
        return false;
    } else {
        return true;
    } 
}

bool hassIsConnected() {
    return clientHass.connected();
}

void hassInit() { 
    if (!isHassEnable()) return;
    clientHass.begin(cfg.hassip.c_str(), cfg.hasspt, netHass);
    clientHass.onMessage(messageReceived);
    mqttHassDelayedStamp = millis() - MQTT_DELAYED_TIME * 1000;
    hassInited = true;
    hassConnect();
}

void hassLoop () {
    if (!isHassEnable()) return;
    if(!WiFi.isConnected()) return; 
    if (!hassInited) hassInit();
    clientHass.loop();
    delay(10);
    if (!clientHass.connected()) hassConnect(); 
    hassPublish();
}