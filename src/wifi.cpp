#include <wifi.hpp>

bool wifiOn;
uint32_t ifxdbwcount;
int rssi = 0;

InfluxArduino influx;
CanAirIoApi api(false);

/******************************************************************************
*   I N F L U X D B   M E T H O D S
******************************************************************************/

bool influxDbIsConfigured() {
    return cfg.ifxdb.length() > 0 && cfg.ifxip.length() > 0 && cfg.dname.length() > 0;
}

void influxDbInit() {
    if (wifiOn && influxDbIsConfigured()) {
        Serial.println("-->[INFLUXDB] connecting..");
        influx.configure(cfg.ifxdb.c_str(), cfg.ifxip.c_str());  //third argument (port number) defaults to 8086
        Serial.print("-->[INFLUXDB] Using HTTPS: ");
        Serial.println(influx.isSecure());  //will be true if you've added the InfluxCert.hpp file.
        cfg.isNewIfxdbConfig = false;       // flag for config via BLE
        delay(1000);
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
    if (millis() - timeStamp > PUBLISH_INTERVAL * 1000) {
        timeStamp = millis();
        if (sensors.isDataReady() && wifiOn && cfg.wifiEnable && cfg.isIfxEnable() && influxDbIsConfigured()) {
            int ifx_retry = 0;
            Serial.printf("-->[INFLUXDB][%s]\n", cfg.dname.c_str());
            Serial.printf("-->[INFLUXDB][%010d] writing to ", ifxdbwcount++);
            Serial.print("" + cfg.ifxip + "..");
            while (!influxDbWrite() && (ifx_retry++ < IFX_RETRY_CONNECTION)) {
                Serial.print(".");
                delay(200);
            }
            if (ifx_retry > IFX_RETRY_CONNECTION) {
                Serial.println("failed!\n-->[E][INFLUXDB] write error, try wifi restart..");
                wifiRestart();
            } else {
                Serial.println("done. [" + String(influx.getResponse()) + "]");
                // st.statusOn(st.bit_cloud);
                // dataSendToggle = true;
                // showDataIcon(true);
                // showUptime(ifxdbwcount);
                delay(200);  // --> because the ESP go to then to light sleep, not remove it!
            }
        }
    }   // else
        // showDataIcon(false);
}

/******************************************************************************
*   C A N A I R I O  A P I   M E T H O D S
******************************************************************************/

bool apiIsConfigured() {
    return cfg.apiusr.length() > 0 && cfg.apipss.length() > 0 && cfg.dname.length() > 0;
}

void apiInit() {
    if (wifiOn && apiIsConfigured()) {
        Serial.println("-->[API] Connecting..");
        // stationId and deviceId, optional endpoint, host and port
        if (cfg.apiuri.equals("") && cfg.apisrv.equals(""))
            api.configure(cfg.dname.c_str(), cfg.deviceId);
        else
            api.configure(cfg.dname.c_str(), cfg.deviceId, cfg.apiuri.c_str(), cfg.apisrv.c_str(), cfg.apiprt);
        api.authorize(cfg.apiusr.c_str(), cfg.apipss.c_str());
        // api.dev = true;
        cfg.isNewAPIConfig = false;  // flag for config via BLE
        delay(1000);
    }
}

void apiLoop() {
    static uint_fast64_t timeStamp = 0;
    if (millis() - timeStamp > PUBLISH_INTERVAL * 1000) {
        timeStamp = millis();
        if (sensors.isDataReady() && wifiOn && cfg.wifiEnable && cfg.isApiEnable() && apiIsConfigured()) {
            Serial.print("-->[API] writing to ");
            Serial.print("" + String(api.ip) + "..");
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
                Serial.println("done. [" + String(code) + "]");
                // st.statusOn(st.bit_cloud);
                // st.dataSendToggle = true;
            } else {
                Serial.println("fail! [" + String(code) + "]");
                // st.statusOff(st.bit_cloud);
                // st.setErrorCode(st.ecode_api_write_fail);
                if (code == -1) {
                    Serial.println("-->[E][API] publish error (-1)");
                    delay(100);
                }
            }
        }
    }
}

/******************************************************************************
*   W I F I   M E T H O D S
******************************************************************************/

class MyOTAHandlerCallbacks: public OTAHandlerCallbacks{
  void onStart(){
    gui.showWelcome();
    gui.welcomeAddMessage("Upgrading..");
  };
  void onProgress(unsigned int progress, unsigned int total){
    gui.showProgress(progress,total);
  };
  void onEnd(){
    gui.welcomeAddMessage("");
    gui.welcomeAddMessage("success!");    delay(1000);
    gui.welcomeAddMessage("rebooting.."); delay(500);
  }
  void onError(){
    gui.welcomeAddMessage("");
    gui.welcomeAddMessage("error, try again!"); delay(2000);
  }
};

void otaLoop() {
    // timerAlarmDisable(timer);  // disable interrupt
    if (wifiOn) ota.loop();
    // timerAlarmEnable(timer);  // enable interrupt
}

void otaInit() {
    ota.setup("CanAirIO", "CanAirIO");
    ota.setCallbacks(new MyOTAHandlerCallbacks());
}

bool wifiCheck() {
    wifiOn = WiFi.isConnected();
    return wifiOn;
}

void wifiConnect(const char* ssid, const char* pass) {
    Serial.print("-->[WIFI] Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, pass);
    int wifi_retry = 0;
    while (!WiFi.isConnected() && wifi_retry++ < WIFI_RETRY_CONNECTION) {
        Serial.print(".");
        delay(100);  // increment this delay on possible reconnect issues
    }
    if (wifiCheck()) {
        cfg.isNewWifi = false;  // flag for config via BLE
        Serial.println("done\n-->[WIFI] connected!");
        Serial.print("-->[WIFI] ");
        Serial.println(WiFi.localIP());
        otaInit();
    } else {
        Serial.println("fail!\n-->[E][WIFI] disconnected!");
    }
}

void wifiInit() {
    if (cfg.wifiEnable && cfg.ssid.length() > 0 && cfg.pass.length() > 0) {
        wifiConnect(cfg.ssid.c_str(), cfg.pass.c_str());
    }
}

void wifiStop() {
    if (wifiOn) {
        Serial.println("-->[WIFI] Disconnecting..");
        WiFi.disconnect(true);
        wifiOn = false;
        delay(1000);
        wifiCheck();
    }
}

void wifiRestart() {
    wifiStop();
    wifiInit();
}

void wifiRSSI(){
  if (wifiOn)
    rssi = WiFi.RSSI();
  else
    rssi = 0;
}

void wifiLoop() {
    static uint_least64_t wifiTimeStamp = 0;
    if (millis() - wifiTimeStamp > 5000  && cfg.wifiEnable && cfg.ssid.length() > 0 && !wifiCheck()) {
        wifiTimeStamp = millis();
        wifiRSSI();
        wifiInit();
        influxDbInit();
        apiInit();
    }
}

int getWifiRSSI(){
    return rssi;
}