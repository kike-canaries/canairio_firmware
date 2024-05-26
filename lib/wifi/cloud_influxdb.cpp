#include <InfluxDbClient.h>
#include "ConfigApp.hpp"
#include "cloud_influxdb.hpp"
#include "wifi.hpp"
#include "power.hpp"

/******************************************************************************
*   I N F L U X D B   M E T H O D S
******************************************************************************/

InfluxDBClient influx;
Point sensor ("fixed_stations_01");
bool ifx_ready;
bool enable_sensors;
int ifx_error_count;

bool influxDbIsConfigured() {
    if(ifx.db.length() > 0 && ifx.ip.length() > 0 && geo.length()==0) {
        Serial.println("[W][IFDB] ifxdb is configured but Location (GeoHash) is missing!");
    }
    return ifx.db.length() > 0 && ifx.ip.length() > 0 && geo.length() > 0;
}

void influxDbAddTags() {
    sensor.addTag("mac",deviceId.c_str());
    sensor.addTag("geo3",geo.substring(0,3).c_str());
    sensor.addTag("name",getStationName().c_str());
    sensor.addTag("rev",getVersion());
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
    sensor.addField("geo",geo.c_str());
    sensor.addField("prs",sensors.getPressure());
    sensor.addField("gas",sensors.getGas());
    sensor.addField("nh3",sensors.getNH3());
    sensor.addField("co",sensors.getCO());
    sensor.addField("no2",sensors.getNO2());
    sensor.addField("alt",sensors.getAltitude());
    sensor.addField("cpm",sensors.getGeigerCPM());
    sensor.addField("usvh",sensors.getGeigerMicroSievertHour());
    sensor.addField("bat",battery.getCharge());
    sensor.addField("vbat",battery.getVoltage());
    sensor.addField("rssi",getWifiRSSI());
    sensor.addField("heap",ESP.getFreeHeap());
    sensor.addField("name",getStationName().c_str());
}

bool influxDbWrite() {
    if(!influxDbIsConfigured()) return false;
    influxDbParseFields();
    log_d("[IFDB] %s",influx.pointToLineProtocol(sensor).c_str());
    if (!influx.writePoint(sensor)) {
        Serial.print("[E][IFDB] Write Point failed: ");
        Serial.println(influx.getLastErrorMessage());
        return false;
    }
    return true;
}

// void suspendDevice() {
//     if (!bleIsConnected()) {
//         if (solarmode && deepSleep > 0) { // sleep mode and ECO mode on
//             powerDeepSleepTimer(deepSleep);
//         }
//         else if (deepSleep > 0) {  // sleep mode, ECO mode off
//             powerDisableSensors();
//             enable_sensors = false;
//         }
//     } else {
//         if (!enable_sensors && !solarmode && deepSleep == 0) { // restore to normal mode
//             powerEnableSensors();
//             enable_sensors = true;
//             sensors.setSampleTime(stime);
//         }
//         if (devmode) Serial.println(F("-->[IFDB] BLE client connected\t: skip shutdown"));
//     }
// }

// void enableSensors() {
//     if (!enable_sensors) {
//         powerEnableSensors();
//         sensors.setSampleTime(deepSleep);
//         sensors.init();
//         enable_sensors = true;
//         Serial.printf("-->[HEAP] sizeof sensors\t: %04ub\r\n", sizeof(sensors));
//     }
// }

void influxDbLoop() {
    static uint_fast64_t timeStamp = 0;
    uint32_t ptime = stime;
    if (ptime<MIN_PUBLISH_INTERVAL) ptime = MIN_PUBLISH_INTERVAL;   // minimum publish interval validation
    // if (solarmode) ptime = MIN_PUBLISH_INTERVAL;
    // if(!solarmode && deepSleep > 0) {
    //     ptime = deepSleep;
    //     if (millis() - timeStamp > (ptime - WAIT_FOR_PM_SENSOR) * 1000) { 
    //         enableSensors(); // enable sensors before publish
    //     } 
    // }
    // if ((solarmode || deepSleep > 0 ) && millis() - timeStamp > (ptime - 2) * 1000) {  
    //     sensors.readAllSensors(); // read sensors after stabilization
    //     delay(500);
    // }
    if (millis() - timeStamp > ptime * 1000) {
        timeStamp = millis();
        if (ifx_ready && sensors.isDataReady() && WiFi.isConnected() && isIfxEnable()) {
            if (influxDbWrite()){
                if(devmode) Serial.printf ("-->[IFDB] CanAirIO published \t: payload size: %d\t:)\r\n", sizeof(sensor));
                gui.displayDataOnIcon();
                // suspendDevice();
                ifx_error_count = 0;
            }
            else {
                Serial.printf("[E][IFDB] write error to %s@%s:%i \r\n",ifx.db.c_str(),ifx.ip.c_str(),ifx.pt);
                if (solarmode && ifx_error_count++ > IFX_ERROR_COUNT_MAX) {
                    powerDeepSleepTimer(deepSleep);
                }
            }
        }
    }  
}

void influxDbInit() {
    if (!ifx_ready && WiFi.isConnected() && isIfxEnable() && influxDbIsConfigured()) {
        String url = "http://" + ifx.ip + ":" + String(ifx.pt);
        influx.setInsecure();
        influx.setConnectionParamsV1(url.c_str(), ifx.db.c_str());
        if (devmode) Serial.printf("-->[IFDB] InfluxDB config  \t: %s:%i\r\n", ifx.ip.c_str(), ifx.pt);
        influxDbAddTags();
        Serial.printf("-->[IFDB] %s\t: ", ifx.ip.c_str());
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