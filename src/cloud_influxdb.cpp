#include <cloud_influxdb.hpp>
#include <wifi.hpp>

/******************************************************************************
*   I N F L U X D B   M E T H O D S
******************************************************************************/

InfluxDBClient influx;
Point sensor ("fixed_stations_01");
bool ifx_ready;

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
    sensor.addField("bat",gui.getBatteryLevel());
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

void influxDbLoop() {
    static uint_fast64_t timeStamp = 0;
    if (millis() - timeStamp > cfg.stime * 2 * 1000) {
        timeStamp = millis();
        if (ifx_ready && sensors.isDataReady() && WiFi.isConnected() && cfg.isIfxEnable()) {
            if (influxDbWrite()){
                if(cfg.devmode) Serial.println("-->[IFDB] write done.");
                gui.displayDataOnIcon();
            }
            else
                Serial.printf("[E][IFDB] write error to %s@%s:%i \n",cfg.ifx.db.c_str(),cfg.ifx.ip.c_str(),cfg.ifx.pt);
        }
    }  
}

void influxDbInit() {
    if (!ifx_ready && WiFi.isConnected() && cfg.isIfxEnable() && influxDbIsConfigured()) {
        String url = "http://" + cfg.ifx.ip + ":" + String(cfg.ifx.pt);
        influx.setInsecure();
        // influx = InfluxDBClient(url.c_str(),cfg.ifx.db.c_str());
        influx.setConnectionParamsV1(url.c_str(), cfg.ifx.db.c_str());
        if (cfg.devmode) Serial.printf("-->[IFDB] config: %s@%s:%i\n", cfg.ifx.db.c_str(), cfg.ifx.ip.c_str(), cfg.ifx.pt);
        influxDbAddTags();
        Serial.printf("-->[IFDB] connecting to: %s..", cfg.ifx.ip.c_str());
        int influx_retry = 0;
        while (influx_retry++ < IFX_RETRY_CONNECTION && !influx.validateConnection()) {
            Serial.print(".");
            delay(100);
        }
        if (influx_retry >= IFX_RETRY_CONNECTION && !influx.validateConnection()) {
            Serial.println("\tconnection failed!");
            Serial.println("[E][IFDB] connection error!");
            return;
        }
        ifx_ready = true;
        Serial.println("\tconnected!");
        delay(10);
    }
}