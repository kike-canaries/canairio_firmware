#include "ConfigApp.hpp"

// std::mutex conf_mtx;

uint64_t chipid;
String deviceId;
String efuseDevId;

trackStatus track;
ifxdbValues ifx;

int stime;
int stype;
int sTX;
int sRX;

bool devmode;
bool pax_enable; 
bool solarmode;
uint32_t deepSleep;
float toffset;
float altoffset;
float sealevel;

bool wifi_enable;
bool ifxdb_enable;
bool wifi_connected;
bool new_wifi = false;

Geohash geohash;

RemoteConfigCallbacks* mRemoteConfigCallBacks = nullptr;

String calcEfuseDeviceId() {
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  return String(chipId, HEX);
}

void init(const char app_name[]) {
    char* _app_name = new char[strlen(app_name) + 1];
    strcpy(_app_name, app_name);
    cfg.init(_app_name);
    chipid = ESP.getEfuseMac();
    deviceId = getDeviceId();
    efuseDevId = calcEfuseDeviceId();
    reload();
    if (devmode) Serial.println("-->[CONF] debug is enable.");
}

void reload() {
    // wifi settings
    wifi_enable = cfg.getBool(CONFKEYS::KWIFIEN, false);
    // influx db optional settings
    ifxdb_enable = cfg.getBool(CONFKEYS::KIFXENB, false);
    ifx.db = cfg.getString(CONFKEYS::KIFXDB, ifx.db);
    ifx.ip = cfg.getString(CONFKEYS::KIFXIP, ifx.ip);
    ifx.pt = cfg.getInt(CONFKEYS::KIFXPT, ifx.pt);
    // station and sensor settings
    stime = cfg.getInt("stime", 5);
    stype = cfg.getInt(CONFKEYS::KSTYPE, 0);
    sTX = cfg.getInt("sTX", -1);
    sRX = cfg.getInt("sRX", -1);
    toffset = cfg.getFloat(CONFKEYS::KTOFFST, 0.0);
    altoffset = cfg.getFloat(CONFKEYS::KALTOFST, 0.0);
    sealevel = cfg.getFloat(CONFKEYS::KSEALVL, 1013.25);
    devmode = cfg.getBool(CONFKEYS::KDEBUG, false);
    pax_enable = cfg.getBool(CONFKEYS::KPAXENB, false);
    solarmode = cfg.getBool(CONFKEYS::KSOLAREN, false);
    deepSleep = cfg.getInt(CONFKEYS::KDEEPSLP, 0); 
}

bool on_read_config = false;

String getCurrentConfig() {
    // std::lock_guard<std::mutex> lck(conf_mtx);
    if (on_read_config) return "";
    on_read_config = true;
    JsonDocument doc;
    doc["wmac"] = (uint16_t)(chipid >> 32);  // chipid calculated in init
    doc["anaireid"] = getStationName();      // deviceId for Anaire cloud
    doc["wsta"] = wifi_connected;            // current wifi state
    doc["vrev"] = REVISION;
    doc["vflv"] = FLAVOR;
    doc["vtag"] = TARGET;
    doc["vmac"] = getDeviceId();
    doc["wenb"] = cfg.getBool(CONFKEYS::KWIFIEN, false);      // wifi on/off
    doc["ienb"] = cfg.getBool(CONFKEYS::KIFXENB, false);      // ifxdb on/off
    doc["denb"] = cfg.getBool(CONFKEYS::KDEBUG, false);       // debug mode enable
    
    doc["ssid"] = cfg.getString(CONFKEYS::KSSID, "");         // influxdb database name
    doc["geo"] = cfg.getString("geo", "");           // influxdb GeoHash tag
    doc["i2conly"] = cfg.getBool(CONFKEYS::KI2CONLY, false);  // force only i2c sensors
    doc["toffset"] = cfg.getFloat(CONFKEYS::KTOFFST, 0.0);    // temperature offset
    doc["stime"] = cfg.getInt(CONFKEYS::KSTIME, 5);           // sensor measure time
    doc["stype"] = cfg.getInt(CONFKEYS::KSTYPE, 0);           // sensor measure time
    doc["ifxdb"] = cfg.getString(CONFKEYS::KIFXDB, ifx.db);   // influxdb database name
    doc["ifxip"] = cfg.getString(CONFKEYS::KIFXIP, ifx.ip);   // influxdb database ip
    doc["ifxpt"] = cfg.getInt(CONFKEYS::KIFXPT, ifx.pt);      // influxdb sensor tags
    doc["hassip"] = cfg.getString(CONFKEYS::KHASSPW, "");     // Home Assistant MQTT server ip
    doc["hasspt"] = cfg.getInt(CONFKEYS::KHASSPT, 1883);      // Home Assistant MQTT server port
    doc["hassusr"] = cfg.getString(CONFKEYS::KHASSUSR, "");   // Home Assistant MQTT user
    doc["sealevel"] = cfg.getFloat(CONFKEYS::KSEALVL,1013.25);// sea level reference
    doc["altoffset"] = cfg.getFloat(CONFKEYS::KALTOFST,0.0);  // CO2 altitude offset

    doc["sse"] = cfg.getBool(CONFKEYS::KSOLAREN, false);      // Enable solar station
    doc["deepSleep"] = cfg.getInt(CONFKEYS::KDEEPSLP, 0);     // deep sleep time in seconds
    String output;
    serializeJson(doc, output);
#if CORE_DEBUG_LEVEL >= 3
    char buf[1000];
    serializeJsonPretty(doc, buf, 1000);
    Serial.printf("-->[CONF] respons@e: %s\r\n", buf);
#endif
    on_read_config = false;
    return output;

//     doc["dname"] = cfg.getString("dname", "");       // device or station name
//     doc["hasspsw"] = cfg.getString("hasspsw", "");// Home Assistant MQTT password
//     doc["sRX"] = cfg.getInt("sRX", -1);           // sensor UART type;
//     doc["sTX"] = cfg.getInt("sTX", -1);           // sensor UART type;
//     doc["penb"] = cfg.getBool(getKey(CONFKEYS::KBPAXENB).c_str(), true);    // PaxCounter enable
//     doc["lskey"] = lastKeySaved;                             // last key saved
//     doc["anaireid"] =  getStationName();                     // deviceId for Anaire cloud

}

bool saveSampleTime(int time) {
    if (time >= 5) {
        cfg.saveInt("stime", time);
        stime=time;
        Serial.printf("-->[CONF] set sample time to\t: %d\r\n", time);
        return true;
    }
    // DEBUG("[W][CONF] warning: sample time is too low!");
    return false;
}

/**
 * @brief saveSensorType
 * @param type UART sensor type. Sync it with Android app
 * @return true (compatibility)
 */
bool saveSensorType(int type) {
    cfg.saveInt(CONFKEYS::KSTYPE, type);
    Serial.printf("-->[CONF] sensor device type\t: %d\r\n", type);
    stype = type;
    return true;
}

/**
 * @brief saveSensorPins
 * @param tx UART sensor TX
 * @param rx UART sensor RX
 * @return true (compatibility)
 */
bool saveSensorPins(int tx, int rx) {
    cfg.saveInt("sTX", tx);
    cfg.saveInt("sRX", rx);
    Serial.printf("-->[CONF] sensor UART TX/RX\t: %d/%d\r\n", tx, rx);
    return true;
}

int getSensorType(){
    return stype;
}

/**
 * @brief saveWifiEnable
 * @param unit save the sensor UNIT selected
 * @return 
 */
bool saveUnitSelected(int unit){
    cfg.saveInt("unit", unit);
    Serial.printf("-->[CONF] default unit to \t: %d\r\n", unit);
    return true;
}

/**
 * @brief getUnitSelected
 * @return unit selected and saved by user (default PM2.5)
 */
int getUnitSelected(){
    return cfg.getInt("unit", 2); 
}

bool saveTempOffset(float offset) {
    cfg.saveFloat(CONFKEYS::KTOFFST, offset);
    Serial.printf("-->[CONF] sensor temp offset\t: %0.2f\r\n", offset);
    return true;
}

bool saveAltitudeOffset(float offset) {
    cfg.saveFloat(CONFKEYS::KALTOFST, offset);
    Serial.printf("-->[CONF] sensor altitude offset\t: %0.2f\r\n", offset);
    if(mRemoteConfigCallBacks!=nullptr) mRemoteConfigCallBacks->onAltitudeOffset(offset);
    return true;
}

bool saveSeaLevel(float hpa) {
    cfg.saveFloat(CONFKEYS::KSEALVL, hpa);
    Serial.printf("-->[CONF] sea level pressure\t: %0.2f\r\n", hpa);
    if(mRemoteConfigCallBacks!=nullptr) mRemoteConfigCallBacks->onSeaLevelPressure(hpa);
    return true;
}

// @deprecated
bool saveSSID(String ssid){
    if (ssid.length() > 0) {
        cfg.saveString(CONFKEYS::KSSID, ssid);
        return true;
    }
    return false;
}

bool saveWifi(String ssid, String pass) {
  if (ssid.length() > 0) {
    cfg.saveBool(CONFKEYS::KWIFIEN, true);
    wifi_enable = true;
    #ifndef DISABLE_CLI
    new_wifi = !wcli.isSSIDSaved(ssid);
    #else
    new_wifi = true;
    #endif
    if (new_wifi) {
      log_i("[CONF] temp saving ssid:%s pass:%s", ssid, pass);
      cfg.saveString(CONFKEYS::KSSID, ssid);
      cfg.saveString(CONFKEYS::KPASS, pass);
    }
    return true; // backward compatibility with the Android app
  }
  log_w("[W][CONF] empty Wifi SSID");
  return false;
}

bool saveCLIWiFi() {
  #ifndef DISABLE_CLI
  if (new_wifi) {
    wcli.setSSID(cfg.getString(CONFKEYS::KSSID, ""));
    wcli.setPASW(cfg.getString(CONFKEYS::KPASS, ""));
    wcli.wifiAPConnect(true);
  }
  delay(200);
  if (!wcli.wifiValidation()) {
    if (new_wifi) wcli.loadAP(wcli.getDefaultAP());
    if (!new_wifi) wcli.loadAP(cfg.getString(CONFKEYS::KSSID, "")); // TODO: check if it's necessary
    String ssid = wcli.getCurrentSSID();
    String pass = wcli.getCurrentPASW();
    log_w("[CONF] restored ssid:%s pass:%s", ssid, pass);
    new_wifi = false;
    return false;
  }
  new_wifi = false;
  #endif
  return true;
}

bool saveInfluxDb(String db, String ip, int pt) {
    if (db.length() > 0 && ip.length() > 0) {
        ifx.db = db;
        ifx.ip = ip;
        ifx.pt = pt;
        cfg.saveString("ifxdb", db);
        cfg.saveString("ifxip", ip);
        if (pt > 0) cfg.saveInt("ifxpt", pt);
        cfg.saveBool(CONFKEYS::KIFXENB, true);
        ifxdb_enable = true;
        Serial.printf("-->[CONF] influxdb: %s@%s:%i\r\n",db.c_str(),ip.c_str(),pt);
        Serial.println("-->[CONF] influxdb config saved.");
        return true;
    }
    log_w("[W][CONF] wrong InfluxDb params!");
    return false;
}

bool saveGeo(double latitude, double longitude, String geoh){
    if (latitude != 0 && longitude != 0) {
        cfg.saveDouble("lat", latitude);
        cfg.saveDouble("lon", longitude);
        cfg.saveString("geo", geoh);
        log_i("[CONF] New Geohash: %s \t: (%.4f,%.4f)\r\n", geoh.c_str(), latitude, longitude);
        return true;
    }
    log_w("[W][CONF] wrong GEO params!");
    return false;
}

bool saveGeo(String geoh){
    if (geoh.length() > 5) {
        float latitude;
        float longitude;
        geohash.decode(geoh.c_str(), geoh.length(), &longitude, &latitude);
        log_i("[CONF] Geohash decoder: %s (%.4f,%.4f)\r\n", geoh, latitude, longitude);
        saveGeo(latitude, longitude, geoh);
        return true;
    }
    log_w("[W][CONF] wrong GEO params!");
    return false;
}

bool wifiEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KWIFIEN, enable);
    wifi_enable = enable;
    Serial.println("-->[CONF] update WiFi state\t: " + String(enable));
    return true;
}

bool ifxdbEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KIFXENB, enable);
    ifxdb_enable = enable;
    Serial.println("-->[CONF] update InfluxDB state\t: " + String(enable));
    return true;
}

bool debugEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KDEBUG, enable);
    devmode = enable;
    Serial.println("-->[CONF] new debug mode\t: " + String(enable));
    return true;
}

bool paxEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KPAXENB, enable);
    pax_enable = enable;
    Serial.println("-->[CONF] new PaxCounter mode\t: " + String(enable));
    return true;
}

bool solarEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KSOLAREN, enable);
    solarmode = enable;
    Serial.println("-->[CONF] Solar Station mode\t: " + String(enable));
    return true;
}

bool saveDeepSleep(int seconds){
    cfg.saveInt(CONFKEYS::KDEEPSLP, seconds);
    deepSleep = seconds;
    Serial.printf("-->[CONF] deep sleep time to\t: %d\r\n", seconds);
    return true;
}

bool saveI2COnly(bool enable) {
    cfg.saveBool(CONFKEYS::KI2CONLY, enable);
    Serial.println("-->[CONF] forced only i2c sensors\t: " + String(enable));
    return true;
}

bool saveHassIP(String ip) {
    cfg.saveString("hassip", ip);
    Serial.printf("-->[CONF] Hass local IP \t: %s saved.\r\n",ip.c_str());
    return true;
}

bool saveHassPort(int port) {
    cfg.saveInt("hasspt", port);
    Serial.printf("-->[CONF] Hass Port  \t: %i saved.\r\n", port);
    return true;
}

bool saveHassUser(String user) {
    cfg.saveString("hassusr", user);
    Serial.printf("-->[CONF] Hass User  \t: %s saved.\r\n", user.c_str());
    return true;
}

bool saveHassPassword(String passw) {
    cfg.saveString("hasspsw", passw);
    if (devmode) Serial.printf("-->[CONF] Hass password %s saved.\r\n", passw.c_str());
    else Serial.println("-->[CONF] Hass password saved.");
    return true;
}

bool save(const char *json) {
    JsonDocument doc;
    auto error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("[E][CONF] deserialize Json failed with code "));
        Serial.println(error.c_str());
        return false;
    }

#if CORE_DEBUG_LEVEL >= 3
    char output[1000];
    serializeJsonPretty(doc, output, 1000);
    Serial.printf("-->[CONF] request: %s\r\n", output);
#endif

    uint16_t cmd = doc["cmd"].as<uint16_t>();
    String act = doc["act"] | "";

    // if (doc.containsKey("dname")) return saveDeviceName(doc["dname"] | "");
    if (doc["stime"].is<int>()) return saveSampleTime(doc["stime"] | 0);
    if (doc["stype"].is<int>()) return saveSensorType(doc["stype"] | 0);
    if (doc["ifxdb"].is<String>()) return saveInfluxDb(doc["ifxdb"] | "", doc["ifxip"] | "", doc["ifxpt"] | 0);
    if (doc["pass"].is<String>() && doc["ssid"].is<String>()) return saveWifi(doc["ssid"] | "", doc["pass"] | "");
    if (doc["ssid"].is<String>()) return saveSSID(doc["ssid"] | "");
    if (doc["geo"].is<String>()) return saveGeo(doc["geo"] | "");
    if (doc["toffset"].is<float>()) return saveTempOffset(doc["toffset"].as<float>());
    if (doc["altoffset"].is<float>()) return saveAltitudeOffset(doc["altoffset"].as<float>());
    if (doc["sealevel"].is<float>()) return saveSeaLevel(doc["sealevel"].as<float>());
    if (doc["hassip"].is<String>()) return saveHassIP(doc["hassip"] | "");
    if (doc["hasspt"].is<int>()) return saveHassPort(doc["hasspt"] | 1883);
    if (doc["hassusr"].is<String>()) return saveHassUser(doc["hassusr"] | "");
    if (doc["hasspsw"].is<String>()) return saveHassPassword(doc["hasspsw"] | "");
    if (doc["deepSleep"].is<int>()) return saveDeepSleep(doc["deepSleep"] | 0);
    
    // some actions with chopid validation (for security reasons)
    if (cmd == ((uint16_t)(chipid >> 32)) && act.length() > 0) {
        if (act.equals("wst")) return wifiEnable(doc["wenb"].as<bool>());
        if (act.equals("ist")) return ifxdbEnable(doc["ienb"].as<bool>());
        if (act.equals("dst")) return debugEnable(doc["denb"].as<bool>());
        if (act.equals("i2c")) return saveI2COnly(doc["i2conly"].as<bool>());
        if (act.equals("pst")) return paxEnable(doc["penb"].as<bool>());
        if (act.equals("sse")) return solarEnable(doc["sse"].as<bool>());
        if (act.equals("cls")) clear();
        if (act.equals("rbt")) reboot();
        if (act.equals("clb")) performCO2Calibration();
        return true;
    } else {
        Serial.println("[E][CONF] invalid config file!");
        return false;
    }
}

bool getTrackStatusValues(const char *json) {
    JsonDocument doc;
    auto error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("[E][CONF] deserialize Json failed with code "));
        Serial.println(error.c_str());
        return false;
    }
    if (doc["spd"].is<float>()) track.spd = doc["spd"] | 0.0;
    if (doc["kms"].is<float>()) track.kms = doc["kms"] | 0.0;
    if (doc["hrs"].is<int>()) track.hrs = doc["hrs"] | 0;
    if (doc["min"].is<int>()) track.min = doc["min"] | 0;
    if (doc["seg"].is<int>()) track.seg = doc["seg"] | 0;

    return true;
}


String getDeviceId() {
    uint8_t baseMac[6];
    // Get MAC address for WiFi station
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char baseMacChr[19] = {0};
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]+2);
    return String(baseMacChr);
}

String getEfuseDeviceId() { 
  return efuseDevId; 
}

String getDeviceIdShort() {
    String devId = getDeviceId();
    devId = devId.substring(13);
    devId.replace(":","");
    return devId;
}

String getStationName() {
    if (cfg.getString("geo", "").isEmpty()) return efuseDevId;
    String name = ""+cfg.getString("geo", "").substring(0,3);          // GeoHash ~70km https://en.wikipedia.org/wiki/Geohash
    String flavor = String(FLAVOR);
    if(flavor.length() > 6) flavor = flavor.substring(0,7); // validation possible issue with HELTEC
    name = name + flavor;                         // Flavor short, firmware name (board)
    name = name + getDeviceId().substring(10);    // MAC address 4 digts
    name.replace("_","");
    name.replace(":","");
    name.toUpperCase();
    return name;
}

String getVersion() {
    return "v"+String(VERSION)+"r"+String(REVISION)+String(TARGET);
}

bool isWifiEnable() {
    return wifi_enable;
}

bool isPaxEnable() {
    return pax_enable;
}

bool isIfxEnable() {
    return ifxdb_enable;
}

void setWifiConnected(bool connected){
    wifi_connected = connected;
}

bool isWifiConnected() {
    return wifi_connected;
}

void clear() {
    cfg.clear();
    Serial.println("-->[CONF] clear settings!");
    delay(200);
    reboot();
}

void reboot() {
    Serial.println("-->[CONF] reboot..");
    delay(100);
    wd.execute();  // ESP and WiFi reboot
}

void performCO2Calibration() {
    if(mRemoteConfigCallBacks!=nullptr) mRemoteConfigCallBacks->onCO2Calibration();
}

void saveBrightness(int value){
    cfg.saveInt("bright",value);
}

int32_t getBrightness(){
    return cfg.getInt("bright",30);
}

void colorsInvertedEnable(bool enable){
    cfg.saveBool("cinverted",enable);
}

void setRemoteConfigCallbacks(RemoteConfigCallbacks* pCallbacks){
    mRemoteConfigCallBacks = pCallbacks;
}
