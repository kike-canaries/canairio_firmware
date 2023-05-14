#include <cloud_hass.hpp>
#include <wifi.hpp>
#include <Batterylib.hpp>

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
    doc["signal_strength"] = String(getWifiRSSI());
    doc["battery"] = String(battery.getCharge());
    doc["voltage"] = String(battery.getVoltage());

    size_t n = serializeJson(doc, buffer);
 
    if (clientHass.publish(getStateTopic().c_str(), buffer, n)) {
        if(cfg.devmode) Serial.printf ("-->[MQTT] HA local published\t: payload size: %d\r\n", n);
    } else {
        Serial.printf("[E][MQTT] HA publish state error\t: %d\r\n",clientHass.lastError());
    }
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
    Serial.printf("[E][MQTT] publish config error\t: class %s (%d)\r\n", dclass.c_str(), clientHass.lastError());
    return false;
}

bool hassRegisterSensors() {
    hassConfigured = publishDiscoveryPayload("temperature", "temperature", "°C");
    hassConfigured = publishDiscoveryPayload("humidity", "humidity", "%");
    hassConfigured = publishDiscoveryPayload("carbon_dioxide", "carbon_dioxide", "ppm");
    hassConfigured = publishDiscoveryPayload("pm25", "pm25", "µg/m³");
    hassConfigured = publishDiscoveryPayload("gas", "gas", "m³");
    hassConfigured = publishDiscoveryPayload("pressure", "pressure", "hPa");
    hassConfigured = publishDiscoveryPayload("battery", "battery", "%");

    if (hassConfigured) Serial.printf("-->[MQTT] HA device registered\t: %s\r\n",getHostId().c_str());
    else Serial.printf("[E][MQTT] HA not configured yet\t: device: %s\r\n",getHostId().c_str());

    return hassConfigured;
}

void messageReceived(String &topic, String &payload) {
  Serial.println("-->[MQTT] incoming msg: " + topic + " - " + payload); 
  if (payload.equals("online")) hassRegisterSensors();
}

bool hassStatusSubscription() {
    if (clientHass.subscribe(getServerStatusTopic().c_str())) return true;
    Serial.printf("[E][MQTT] status subscription error\t: %d\r\n",clientHass.lastError());
    hassSubscribed = false;
    return false;
}

void hassPublish() {
    if (!clientHass.connected()) return;
    static uint_fast64_t mqttTimeStamp = 0;
    uint32_t ptime = cfg.stime;
    if (ptime<MIN_PUBLISH_INTERVAL) ptime = MIN_PUBLISH_INTERVAL-1; // publish before to the last cloud
    if(!cfg.solarmode && cfg.deepSleep > 0) ptime = cfg.deepSleep;
    if (millis() - mqttTimeStamp > ptime * 1000) {
        mqttTimeStamp = millis(); 
        if (!hassConfigured) hassRegisterSensors();
        hassPubSensorPayload();
        if (!hassSubscribed) hassStatusSubscription();
    }
}

bool hassAuth() {
    return clientHass.connect(cfg.getStationName().c_str(), cfg.hassusr.c_str(), cfg.hasspsw.c_str());
}

static uint_fast64_t mqttHassDelayedStamp = 0;

void hassConnect() {
    if (!(cfg.isWifiEnable() && WiFi.isConnected())) return;
    if (millis() - mqttHassDelayedStamp > MQTT_DELAYED_TIME * 1000) {
        Serial.printf("-->[MQTT] %s\t: ", cfg.hassip.c_str());
        int mqtt_try = 0;
        while (mqtt_try++ < MQTT_RETRY_CONNECTION && !hassAuth()) {
            delay(100);
        }
        if (mqtt_try >= MQTT_RETRY_CONNECTION && !clientHass.connected()) {
            mqttHassDelayedStamp = millis();
            hassSubscribed = false;
            hassConfigured = false;
            Serial.println("connection failed!");
            if (cfg.devmode) Serial.printf("-->[MQTT] %s\r\n",cfg.hassusr.c_str());
            return;
        }
        Serial.println("connected!");
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
    if(!WiFi.isConnected()) return;
    if (!isHassEnable()) return;
    if (!clientHass.connected()) {
      anaireInit();
      delay(10);
    }
    clientHass.loop();
    delay(10);
    if (!clientHass.connected()) hassConnect(); 
    hassPublish();
}