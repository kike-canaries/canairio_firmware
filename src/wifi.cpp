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
        Serial.println("-->[W][IFDB] ifxdb is configured but device name is missing!");
    }
    return cfg.ifx.db.length() > 0 && cfg.ifx.ip.length() > 0 && cfg.dname.length() > 0;
}

void influxDbInit() {
    if (WiFi.isConnected() && cfg.isIfxEnable() && influxDbIsConfigured()) {
        influx.configure(cfg.ifx.db.c_str(), cfg.ifx.ip.c_str(),cfg.ifx.pt);  //third argument (port number) defaults to 8086
        Serial.print("-->[IFDB] using HTTPS: ");
        Serial.println(influx.isSecure());  //will be true if you've added the InfluxCert.hpp file.
        cfg.isNewIfxdbConfig = false;       // flag for config via BLE
        Serial.println("-->[IFDB] connected.");
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
    // select humi and temp for publish it
    float humi = sensors.getHumidity();
    if(humi == 0.0) humi = sensors.getCO2humi();
    float temp = sensors.getTemperature();
    if(temp == 0.0) temp = sensors.getCO2temp();

    sprintf(
        fields,
        "pm1=%u,pm25=%u,pm10=%u,co2=%u,co2hum=%f,co2tmp=%f,hum=%f,tmp=%f,prs=%f,gas=%f,lat=%f,lng=%f,alt=%f,stime=%i",
        sensors.getPM1(),
        sensors.getPM25(),
        sensors.getPM10(),
        sensors.getCO2(),
        sensors.getCO2humi(),
        sensors.getCO2temp(),
        humi,
        temp,
        sensors.getPressure(),
        sensors.getGas(),
        cfg.lat,
        cfg.lon,
        sensors.getAltitude(),
        cfg.stime
    );
}

void influxDbAddTags(char* tags) {
    sprintf(tags, "mac=%s,dtype=%s",cfg.deviceId.c_str(),sensors.getPmDeviceSelected().c_str());
}

bool influxDbWrite() {
    char tags[1024];
    influxDbAddTags(tags);
    log_i("[IFDB] Adding tags: %s", tags);
    char fields[1024];
    influxDbParseFields(fields);
    log_i("[IFDB] Adding fields: %s", fields);
    return influx.write(cfg.dname.c_str(), tags, fields);
}

void influxDbLoop() {
    static uint_fast64_t timeStamp = 0;
    if (millis() - timeStamp > cfg.stime * 2 * 1000) {
        timeStamp = millis();
        if (sensors.isDataReady() && WiFi.isConnected() && cfg.isWifiEnable() && cfg.isIfxEnable() && influxDbIsConfigured()) {
            int ifx_retry = 0;
            log_i("[IFDB][ %s ]", cfg.dname.c_str());
            log_i("[IFDB][ %010d ] writing to %s", ifxdbwcount++, cfg.ifx.ip.c_str());
            while (!influxDbWrite() && (ifx_retry++ < IFX_RETRY_CONNECTION)) {
                delay(200);
            }
            if (ifx_retry > IFX_RETRY_CONNECTION) {
                Serial.printf("-->[E][IFDB] !! write error with config: %s@%s:%i !!\n",cfg.ifx.db.c_str(),cfg.ifx.ip.c_str(),cfg.ifx.pt);
                Serial.println("-->[E][IFDB] !! try wifi restart.. !!");
                wifiRestart();
            } else {
                if(cfg.devmode) Serial.println("-->[IFDB] write done.");
                log_i("[IFDB] write done. Response: %d", influx.getResponse());
                gui.displayDataOnIcon();
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

void onUpdateMessage(const char *msg){
    gui.welcomeAddMessage("Updating to:");
    gui.welcomeAddMessage(msg);
    gui.welcomeAddMessage("please wait..");
}

void otaInit() {
    ota.setup("CanAirIO", "CanAirIO");
    ota.setCallbacks(new MyOTAHandlerCallbacks());
    ota.setOnUpdateMessageCb(&onUpdateMessage);
}

void wifiConnect(const char* ssid, const char* pass) {
    Serial.print("-->[WIFI] connecting to ");
    Serial.print(ssid);
    int wifi_retry = 0;
    WiFi.begin(ssid, pass);
    while (!WiFi.isConnected() && wifi_retry++ < WIFI_RETRY_CONNECTION) {
        Serial.print(".");
        delay(100);  // increment this delay on possible reconnect issues
    }
    if (WiFi.isConnected()) {
        cfg.isNewWifi = false;  // flag for config via BLE
        Serial.println(" done.");
        Serial.println("-->[WIFI] connected!");
        Serial.print("-->[WIFI] ");
        Serial.println(WiFi.localIP());
        Serial.println("-->[WIFI] publish interval: "+String(cfg.stime * 2)+" sec.");
        wd.pause();
        otaInit();
        ota.checkRemoteOTA();
        wd.resume();
    } else {
        Serial.println("fail!\n-->[E][WIFI] disconnected!");
    }
}

void wifiInit() {
    if (cfg.isWifiEnable() && cfg.ssid.length() > 0 ){
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
    if (millis() - wifiTimeStamp > 5000) {
        wifiTimeStamp = millis();
        if (cfg.isWifiEnable() && cfg.ssid.length() > 0 && !WiFi.isConnected()) {
            wifiInit();
            influxDbInit();
        }
        cfg.setWifiConnected(WiFi.isConnected());
    }
}

int getWifiRSSI() {
    if (WiFi.isConnected()) return WiFi.RSSI();
    else return 0;
}