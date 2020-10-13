#include <wifi.hpp>

uint32_t ifxdbwcount;
int rssi = 0;

InfluxArduino influx;
CanAirIoApi api(false);

/******************************************************************************
*   I N F L U X D B   M E T H O D S
******************************************************************************/

bool influxDbIsConfigured() {
    if(cfg.ifx.db.length() > 0 && cfg.ifx.ip.length() > 0 && cfg.dname.length()==0) {
        Serial.println("-->[W][INFLUXDB] ifxdb is configured but device name is missing!");
    }
    return cfg.ifx.db.length() > 0 && cfg.ifx.ip.length() > 0 && cfg.dname.length() > 0;
}

void influxDbInit() {
    if (WiFi.isConnected() && influxDbIsConfigured()) {
        Serial.println("-->[INFLUXDB] connecting..");
        influx.configure(cfg.ifx.db.c_str(), cfg.ifx.ip.c_str());  //third argument (port number) defaults to 8086
        Serial.print("-->[INFLUXDB] Using HTTPS: ");
        Serial.println(influx.isSecure());  //will be true if you've added the InfluxCert.hpp file.
        cfg.isNewIfxdbConfig = false;       // flag for config via BLE
        delay(100);
    }
}

/**
 * @influxDbParseFields:
 *
 * Supported:
 * "id","pm1","pm25","pm10,"hum","tmp","lat","lng","alt","spd","stime","tstp"
 *
 */
void influxDbParseFields(char* fields) {
    sprintf(
        fields,
        "pm1=%u,pm25=%u,pm10=%u,hum=%f,tmp=%f,prs=%f,gas=%f,lat=%f,lng=%f,alt=%f,spd=%f,stime=%i,tstp=%u",
        sensors.getPM1(),
        sensors.getPM25(),
        sensors.getPM10(),
        sensors.getHumidity(),
        sensors.getTemperature(),
        sensors.getPressure(),
        sensors.getGas(),
        cfg.lat,
        cfg.lon,
        sensors.getAltitude(),
        cfg.spd,
        cfg.stime,
        0);
}

void influxDbAddTags(char* tags) {
    sprintf(tags, "mac=%04X%08X", (uint16_t)(cfg.chipid >> 32), (uint32_t)cfg.chipid);
}

bool influxDbWrite() {
    char tags[64];
    influxDbAddTags(tags);
    char fields[256];
    influxDbParseFields(fields);
    return influx.write(cfg.dname.c_str(), tags, fields);
}

void influxDbLoop() {
    static uint_fast64_t timeStamp = 0;
    if (millis() - timeStamp > cfg.stime * 2 * 1000) {
        timeStamp = millis();
        if (sensors.isDataReady() && WiFi.isConnected() && cfg.isWifiEnable() && cfg.isIfxEnable() && influxDbIsConfigured()) {
            int ifx_retry = 0;
            log_i("[INFLUXDB][ %s ]", cfg.dname.c_str());
            log_i("[INFLUXDB][ %010d ] writing to %s", ifxdbwcount++, cfg.ifx.ip.c_str());
            while (!influxDbWrite() && (ifx_retry++ < IFX_RETRY_CONNECTION)) {
                delay(200);
            }
            if (ifx_retry > IFX_RETRY_CONNECTION) {
                Serial.println("-->[E][INFLUXDB] write error, try wifi restart..");
                wifiRestart();
            } else {
                log_i("[INFLUXDB] write done. Response: %d", influx.getResponse());
                gui.displayDataOnIcon();
            }
        }
    }  
}

/******************************************************************************
*   C A N A I R I O  A P I   M E T H O D S
******************************************************************************/

bool apiIsConfigured() {
    return cfg.apiusr.length() > 0 && cfg.apipss.length() > 0 && cfg.dname.length() > 0;
}

void apiInit() {
    if (WiFi.isConnected() && apiIsConfigured() && cfg.isApiEnable()) {
        Serial.println("-->[API] connecting..");
        // stationId and deviceId, optional endpoint, host and port
        if (cfg.apiuri.equals("") && cfg.apisrv.equals(""))
            api.configure(cfg.dname.c_str(), cfg.deviceId.c_str());
        else
            api.configure(cfg.dname.c_str(), cfg.deviceId.c_str(), cfg.apiuri.c_str(), cfg.apisrv.c_str(), cfg.apiprt);
        api.authorize(cfg.apiusr.c_str(), cfg.apipss.c_str());
        // api.dev = true;
        cfg.isNewAPIConfig = false;  // flag for config via BLE
        delay(100);
    }
}

void apiLoop() {
    static uint_fast64_t timeStamp = 0;
    if (millis() - timeStamp > cfg.stime * 2 * 1000) {
        timeStamp = millis();
        if (sensors.isDataReady() && WiFi.isConnected() && cfg.isWifiEnable() && cfg.isApiEnable() && apiIsConfigured()) {
            log_i("[API] writing to %s", api.ip);
            bool status = api.write(
                sensors.getPM1(),
                sensors.getPM25(),
                sensors.getPM10(),
                sensors.getHumidity(),
                sensors.getTemperature(),
                cfg.lat,
                cfg.lon,
                cfg.alt,
                cfg.spd,
                cfg.stime);
            int code = api.getResponse();
            if (status) {
                log_i("done. [%d]",code);
                gui.displayDataOnIcon();
            } else {
                Serial.println("-->[E][API] write error!");
                if (code == -1) Serial.println("-->[E][API] publish error (-1)");
            }
        }
    }
}

/******************************************************************************
*   W I F I   M E T H O D S
******************************************************************************/

class MyOTAHandlerCallbacks : public OTAHandlerCallbacks {
    void onStart() {
        gui.showWelcome();
        gui.welcomeAddMessage("Upgrading..");
    };
    void onProgress(unsigned int progress, unsigned int total) {
        gui.showProgress(progress, total);
    };
    void onEnd() {
        gui.welcomeAddMessage("");
        gui.welcomeAddMessage("success!");
        delay(1000);
        gui.welcomeAddMessage("rebooting..");
        delay(500);
    }
    void onError() {
        gui.welcomeAddMessage("");
        gui.welcomeAddMessage("error, try again!");
        delay(2000);
    }
};

void otaLoop() {
    if (WiFi.isConnected()) {
        wd.pause();
        ota.loop();
        wd.resume();
    }
}

void otaInit() {
    ota.setup("CanAirIO", "CanAirIO");
    ota.setCallbacks(new MyOTAHandlerCallbacks());
}

void wifiConnect(const char* ssid, const char* pass) {
    Serial.print("-->[WIFI] connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, pass);
    int wifi_retry = 0;
    while (!WiFi.isConnected() && wifi_retry++ < WIFI_RETRY_CONNECTION) {
        Serial.print(".");
        delay(100);  // increment this delay on possible reconnect issues
    }
    if (WiFi.isConnected()) {
        cfg.isNewWifi = false;  // flag for config via BLE
        Serial.println("done.\n-->[WIFI] connected!");
        Serial.print("-->[WIFI] ");
        Serial.println(WiFi.localIP());
        Serial.println("-->[WIFI] publish interval: "+String(cfg.stime * 2)+" sec.");
        otaInit();
    } else {
        Serial.println("fail!\n-->[E][WIFI] disconnected!");
    }
}

void wifiInit() {
    if (cfg.isWifiEnable() && cfg.ssid.length() > 0 && cfg.pass.length() > 0) {
        wifiConnect(cfg.ssid.c_str(), cfg.pass.c_str());
    }
}

void wifiStop() {
    if (WiFi.isConnected()) {
        Serial.println("-->[WIFI] Disconnecting..");
        WiFi.disconnect(true);
        delay(100);
    }
}

void wifiRestart() {
    wifiStop();
    wifiInit();
}

void wifiLoop() {
    static uint_least64_t wifiTimeStamp = 0;
    if (millis() - wifiTimeStamp > 5000  && cfg.isWifiEnable() && cfg.ssid.length() > 0 && !WiFi.isConnected()) {
        wifiTimeStamp = millis();
        wifiInit();
        influxDbInit();
        apiInit();
    }
}

int getWifiRSSI() {
    if (WiFi.isConnected()) return WiFi.RSSI();
    else return 0;
}