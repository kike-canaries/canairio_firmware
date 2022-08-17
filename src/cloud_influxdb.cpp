#include <cloud_influxdb.hpp>
#include <wifi.hpp>
#include <Batterylib.hpp>
#include <bluetooth.hpp>

/******************************************************************************
*   I N F L U X D B   M E T H O D S
******************************************************************************/

InfluxDBClient influx;
Point sensor ("fixed_stations_01");
bool ifx_ready;
bool enable_sensors;
int ifx_error_count;

bool influxDbIsConfigured() {
    if(cfg.ifx.db.length() > 0 && cfg.ifx.ip.length() > 0 && cfg.geo.length()==0) {
        Serial.println("[W][IFDB] ifxdb is configured but Location (GeoHash) is missing!");
    }
    return cfg.ifx.db.length() > 0 && cfg.ifx.ip.length() > 0 && cfg.geo.length() > 0;
}

void influxDbAddTags() {
    sensor.addTag("mac",cfg.deviceId.c_str());
    sensor.addTag("geo3",cfg.geo.substring(0,3).c_str());
    sensor.addTag("name",cfg.getStationName().c_str());
    sensor.addTag("rev","v"+String(VERSION)+"r"+String(REVISION)+String(TARGET));
}

void influxDbParseFields() {
    sensor.clearFields();

    float humi = sensors.getHumidity();
    if (humi == 0.0) humi = sensors.getCO2humi();
    float temp = sensors.getTemperature();
    if (temp == 0.0) temp = sensors.getCO2temp();

    sensor.addField("pm1",sensors.getPM1());
    sensor.addField("pm25",sensors.getPM25());
    sensor.addField("pm10",sensors.getPM10());
    sensor.addField("co2",sensors.getCO2());
    sensor.addField("co2hum",sensors.getCO2humi());
    sensor.addField("co2tmp",sensors.getCO2temp());
    sensor.addField("tmp",temp);
    sensor.addField("hum",humi);
    sensor.addField("geo",cfg.geo.c_str());
    sensor.addField("prs",sensors.getPressure());
    sensor.addField("gas",sensors.getGas());
    sensor.addField("alt",sensors.getAltitude());
    sensor.addField("bat",battery.getCharge());
    sensor.addField("vbat",battery.getVoltage());
    sensor.addField("rssi",getWifiRSSI());
    sensor.addField("heap",ESP.getFreeHeap());
    sensor.addField("name",cfg.getStationName().c_str());
}

bool influxDbWrite() {
    influxDbParseFields();
    log_d("[IFDB] %s",influx.pointToLineProtocol(sensor).c_str());
    if (!influx.writePoint(sensor)) {
        Serial.print("[E][IFDB] Write Point failed: ");
        Serial.println(influx.getLastErrorMessage());
        return false;
    }
    return true;
}

void suspendDevice() {
    if (!bleIsConnected()) {
        if (cfg.solarmode && cfg.deepSleep > 0) { // sleep mode and ECO mode on
            powerDeepSleepTimer(cfg.deepSleep);
        }
        else if (cfg.deepSleep > 0) {  // sleep mode, ECO mode off
            powerDisableSensors();
            enable_sensors = false;
        }
    } else {
        if (!enable_sensors && !cfg.solarmode && cfg.deepSleep == 0) { // restore to normal mode
            powerEnableSensors();
            enable_sensors = true;
            sensors.setSampleTime(cfg.stime);
        }
        if (cfg.devmode) Serial.println(F("-->[IFDB] BLE client connected\t: skip shutdown"));
    }
}

void enableSensors() {
    if (!enable_sensors) {
        powerEnableSensors();
        sensors.setSampleTime(cfg.deepSleep);
        sensors.init();
        enable_sensors = true;
        Serial.printf("-->[HEAP] sizeof sensors\t: %04ub\n", sizeof(sensors));
    }
}

void influxDbLoop() {
    static uint_fast64_t timeStamp = 0;
    uint32_t ptime = cfg.stime;
    if (ptime<MIN_PUBLISH_INTERVAL) ptime = MIN_PUBLISH_INTERVAL;   // minimum publish interval validation
    if (cfg.solarmode) ptime = MIN_PUBLISH_INTERVAL;
    if(!cfg.solarmode && cfg.deepSleep > 0) {
        ptime = cfg.deepSleep;
        if (millis() - timeStamp > (ptime - WAIT_FOR_PM_SENSOR) * 1000) { 
            enableSensors(); // enable sensors before publish
        } 
    }
    if ((cfg.solarmode || cfg.deepSleep > 0 ) && millis() - timeStamp > (ptime - 2) * 1000) {  
        sensors.readAllSensors(); // read sensors after stabilization
        delay(500);
    }
    if (millis() - timeStamp > ptime * 1000) {
        timeStamp = millis();
        if (ifx_ready && sensors.isDataReady() && WiFi.isConnected() && cfg.isIfxEnable()) {
            if (influxDbWrite()){
                if(cfg.devmode) Serial.printf ("-->[IFDB] CanAirIO cloud write\t: payload size: %d\n", sizeof(sensor));
                gui.displayDataOnIcon();
                suspendDevice();
                ifx_error_count = 0;
            }
            else {
                Serial.printf("[E][IFDB] write error to %s@%s:%i \n",cfg.ifx.db.c_str(),cfg.ifx.ip.c_str(),cfg.ifx.pt);
                if (cfg.solarmode && ifx_error_count++ > IFX_ERROR_COUNT_MAX) {
                    powerDeepSleepTimer(cfg.deepSleep);
                }
            }
        }
    }  
}

void influxDbInit() {
    if (!ifx_ready && WiFi.isConnected() && cfg.isIfxEnable() && influxDbIsConfigured()) {
        String url = "http://" + cfg.ifx.ip + ":" + String(cfg.ifx.pt);
        influx.setInsecure();
        influx.setConnectionParamsV1(url.c_str(), cfg.ifx.db.c_str());
        if (cfg.devmode) Serial.printf("-->[IFDB] InfluxDB config  \t: %s:%i\n", cfg.ifx.ip.c_str(), cfg.ifx.pt);
        influxDbAddTags();
        Serial.printf("-->[IFDB] %s\t: ", cfg.ifx.ip.c_str());
        int influx_retry = 0;
        while (influx_retry++ < IFX_RETRY_CONNECTION && !influx.validateConnection()) {
            delay(100);
        }
        if (influx_retry >= IFX_RETRY_CONNECTION && !influx.validateConnection()) {
            Serial.println("connection failed!");
            Serial.println("[E][IFDB] connection error!");
            return;
        }
        ifx_ready = true;
        Serial.println("connected!");
        delay(10);
    }
}