#include "ConfigApp.hpp"

uint64_t chipid;
String deviceId;
String dname;

trackStatus track;
ifxdbValues ifx;

int stime;
int stype;
int sTX;
int sRX;
double lat;
double lon;
String geo;

String ssid;
String pass;

String hassip;
String hassusr;
String hasspsw;
int16_t hasspt;

bool isNewWifi;
bool devmode;
bool i2conly;
bool pax_enable; 
bool solarmode;
uint32_t deepSleep;
float toffset;
float altoffset;
float sealevel;

char* _app_name;
bool wifi_enable;
bool ifxdb_enable;
bool wifi_connected;

Geohash geohash;

RemoteConfigCallbacks* mRemoteConfigCallBacks = nullptr;

void init(const char app_name[]) {
    _app_name = new char[strlen(app_name) + 1];
    strcpy(_app_name, app_name);
    cfg.init(_app_name);
    chipid = ESP.getEfuseMac();
    deviceId = getDeviceId();
    reload();
    // override with debug INFO level (>=3)
#if CORE_DEBUG_LEVEL >= 1
    devmode = true;
#endif
    if (devmode) Serial.println("-->[CONF] debug is enable.");
}

void reload() {
    // device name or station name
    // dname = cfg.getString("dname", "");
    // wifi settings
    wifi_enable = cfg.getBool(CONFKEYS::KBWIFIEN, false);
    ssid = cfg.getString(CONFKEYS::KSSID, "");
    pass = cfg.getString(CONFKEYS::KPASS, "");
    // influx db optional settings
    ifxdb_enable = cfg.getBool(CONFKEYS::KBIFXENB, false);
    ifx.db = cfg.getString(CONFKEYS::KSIFXDB, ifx.db);
    ifx.ip = cfg.getString(CONFKEYS::KSIFXIP, ifx.ip);
    ifx.pt = cfg.getInt(CONFKEYS::KIIFXPT, ifx.pt);
    // station and sensor settings
    lat = cfg.getDouble("lat", 0);
    lon = cfg.getDouble("lon", 0);
    geo = cfg.getString("geo", "");
    stime = cfg.getInt("stime", 5);
    stype = cfg.getInt("stype", 0);
    sTX = cfg.getInt("sTX", -1);
    sRX = cfg.getInt("sRX", -1);
    toffset = cfg.getFloat("toffset", 0.0);
    altoffset = cfg.getFloat("altoffset", 0.0);
    sealevel = cfg.getFloat("sealevel", 1013.25);
    devmode = cfg.getBool("debugEnable", false);
    pax_enable = cfg.getBool(CONFKEYS::KBPAXENB, true);
    i2conly = cfg.getBool("i2conly", false);
    solarmode = cfg.getBool("solarEnable", false);
    deepSleep = cfg.getInt("deepSleep", 0);
    hassip = cfg.getString("hassip", "");
    hasspt = cfg.getInt("hasspt", 1883);
    hassusr = cfg.getString("hassusr", "");
    hasspsw = cfg.getString("hasspsw", "");
}

String getCurrentConfig() {
    StaticJsonDocument<1000> doc;
    doc["dname"] = cfg.getString("dname", "");       // device or station name
    doc["stime"] = cfg.getInt("stime", 5);           // sensor measure time
    doc["stype"] = cfg.getInt("stype", 0);           // sensor UART type;
    doc["sRX"] = cfg.getInt("sRX", -1);           // sensor UART type;
    doc["sTX"] = cfg.getInt("sTX", -1);           // sensor UART type;
    doc["wenb"] = cfg.getBool(CONFKEYS::KBWIFIEN, false);  // wifi on/off
    doc["ssid"] = cfg.getString("ssid", "");         // influxdb database name
    doc["ienb"] = cfg.getBool(CONFKEYS::KBIFXENB, false);   // ifxdb on/off
    doc["ifxdb"] = cfg.getString("ifxdb", ifx.db);   // influxdb database name
    doc["ifxip"] = cfg.getString("ifxip", ifx.ip);   // influxdb database ip
    doc["ifxpt"] = cfg.getInt("ifxpt", ifx.pt);     // influxdb sensor tags
    doc["geo"] = cfg.getString("geo", "");           // influxdb GeoHash tag
    doc["denb"] = cfg.getBool("debugEnable", false); // debug mode enable
    doc["penb"] = cfg.getBool(CONFKEYS::KBPAXENB, true);    // PaxCounter enable
    doc["i2conly"] = cfg.getBool("i2conly", false);  // force only i2c sensors
    doc["sse"] = cfg.getBool("solarEnable", false);  // Enable solar station
    doc["deepSleep"] = cfg.getInt("deepSleep", 0);  // deep sleep time in seconds
    doc["toffset"] = cfg.getFloat("toffset", 0.0);   // temperature offset
    doc["altoffset"] = cfg.getFloat("altoffset",0.0);// altitude offset
    doc["sealevel"] = cfg.getFloat("sealevel",1013.25);// altitude offset
    doc["hassip"] = cfg.getString("hassip", "");     // Home Assistant MQTT server ip
    doc["hasspt"] = cfg.getInt("hasspt", 1883);      // Home Assistant MQTT server port
    doc["hassusr"] = cfg.getString("hassusr", "");   // Home Assistant MQTT user
    // doc["hasspsw"] = cfg.getString("hasspsw", "");// Home Assistant MQTT password
    doc["wmac"] = (uint16_t)(chipid >> 32);                  // chipid calculated in init
    doc["anaireid"] =  getStationName();                     // deviceId for Anaire cloud
    doc["wsta"] = wifi_connected;                            // current wifi state 
    doc["vrev"] = REVISION;
    doc["vflv"] = FLAVOR;
    doc["vtag"] = TARGET;
    doc["vmac"] = getDeviceId();
    String output;
    serializeJson(doc, output);
#if CORE_DEBUG_LEVEL >= 3
    char buf[1000];
    serializeJsonPretty(doc, buf, 1000);
    Serial.printf("-->[CONF] response: %s", buf);
    Serial.println("");
#endif
    return output;
}



// /**
//  * @brief DEPRECATED
//  */ 
// bool saveDeviceName(String name) {
//     if (name.length() > 0) {
//         saveString("dname",name);
//         Serial.println("-->[CONF] set device name to: " + name);
//         return true;
//     }
//     DEBUG("[E][CONF] device name is empty!");
//     return false;
// }

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
    cfg.saveInt("stype", type);
    Serial.printf("-->[CONF] sensor device type\t: %d\r\n", type);
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
    cfg.saveFloat("toffset", offset);
    Serial.printf("-->[CONF] sensor temp offset\t: %0.2f\r\n", offset);
    return true;
}

bool saveAltitudeOffset(float offset) {
    cfg.saveFloat("altoffset", offset);
    Serial.printf("-->[CONF] sensor altitude offset\t: %0.2f\r\n", offset);
    if(mRemoteConfigCallBacks!=nullptr) mRemoteConfigCallBacks->onAltitudeOffset(offset);
    return true;
}

bool saveSeaLevel(float hpa) {
    cfg.saveFloat("sealevel", hpa);
    Serial.printf("-->[CONF] sea level pressure\t: %0.2f\r\n", hpa);
    if(mRemoteConfigCallBacks!=nullptr) mRemoteConfigCallBacks->onSeaLevelPressure(hpa);
    return true;
}

bool saveSSID(String ssid){
    if (ssid.length() > 0) {
        cfg.saveString("ssid", ssid);
        Serial.println("-->[CONF] WiFi SSID saved!");
        return true;
    }
    // DEBUG("[W][CONF] empty Wifi SSID");
    return false;
}

bool saveWifi(String ssid, String pass){
    if (ssid.length() > 0) {
        cfg.saveString("ssid", ssid);
        cfg.saveString("pass", pass);
        cfg.saveBool(CONFKEYS::KBWIFIEN, true);
        wifi_enable = true;
        isNewWifi = true;  // for execute wifi reconnect
        Serial.println("-->[CONF] WiFi credentials saved!");
        log_i("[CONF] ssid:%s pass:%s",ssid,pass);
        return true;
    }
    // DEBUG("[W][CONF] empty Wifi SSID");
    return false;
}

bool saveInfluxDb(String db, String ip, int pt) {
    if (db.length() > 0 && ip.length() > 0) {
        cfg.saveString("ifxdb", db);
        cfg.saveString("ifxip", ip);
        if (pt > 0) cfg.saveInt("ifxpt", pt);
        cfg.saveBool(CONFKEYS::KBIFXENB, true);
        ifxdb_enable = true;
        Serial.printf("-->[CONF] influxdb: %s@%s:%i\r\n",db.c_str(),ip.c_str(),pt);
        Serial.println("-->[CONF] influxdb config saved.");
        return true;
    }
    // DEBUG("[W][CONF] wrong InfluxDb params!");
    return false;
}

bool saveGeo(double lat, double lon, String geo){
    if (lat != 0 && lon != 0) {
        cfg.saveDouble("lat", lat);
        cfg.saveDouble("lon", lon);
        cfg.saveString("geo", geo);
        lat = lat;
        lon = lon;
        geo = geo;
        Serial.printf("-->[CONF] New Geohash: %s \t: (%.4f,%.4f)\r\n",geo,lat,lon);
        return true;
    }
    // DEBUG("[W][CONF] wrong GEO params!");
    return false;
}

bool saveGeo(String geo){
    if (geo.length() > 5) {
        float lat;
        float lon;
        geohash.decode(geo.c_str(),geo.length(),&lon,&lat);
        log_i("[CONF] Geohash decoder: %s (%.4f,%.4f)\r\n",geo,lat,lon);
        saveGeo(lat,lon, geo);
        return true;
    }
    // DEBUG("[W][CONF] wrong GEO params!");
    return false;
}

bool wifiEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KBWIFIEN, enable);
    wifi_enable = enable;
    Serial.println("-->[CONF] updating WiFi state\t: " + String(enable));
    return true;
}

bool ifxdbEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KBIFXENB, enable);
    ifxdb_enable = enable;
    Serial.println("-->[CONF] updating InfluxDB state\t: " + String(enable));
    return true;
}

bool debugEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KDEBUG, enable);
    devmode = enable;
    Serial.println("-->[CONF] new debug mode\t: " + String(enable));
    return true;
}

bool paxEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KBPAXENB, enable);
    pax_enable = enable;
    Serial.println("-->[CONF] new PaxCounter mode\t: " + String(enable));
    return true;
}

bool solarEnable(bool enable) {
    cfg.saveBool(CONFKEYS::KBSOLARE, enable);
    solarmode = enable;
    Serial.println("-->[CONF] Solar Station mode\t: " + String(enable));
    return true;
}

bool saveDeepSleep(int seconds){
    cfg.saveInt(CONFKEYS::KIDEEPSL, seconds);
    deepSleep = seconds;
    Serial.printf("-->[CONF] deep sleep time to\t: %d\r\n", seconds);
    return true;
}

bool saveI2COnly(bool enable) {
    cfg.saveBool(CONFKEYS::KBI2COLY, enable);
    i2conly = enable;
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
    StaticJsonDocument<1000> doc;
    auto error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("[E][CONF] deserialize Json failed with code "));
        Serial.println(error.c_str());
        return false;
    }

#if CORE_DEBUG_LEVEL >= 3
    char output[1000];
    serializeJsonPretty(doc, output, 1000);
    Serial.printf("-->[CONF] request: %s", output);
    Serial.println("");
#endif

    uint16_t cmd = doc["cmd"].as<uint16_t>();
    String act = doc["act"] | "";

    // if (doc.containsKey("dname")) return saveDeviceName(doc["dname"] | "");
    if (doc.containsKey("stime")) return saveSampleTime(doc["stime"] | 0);
    if (doc.containsKey("stype")) return saveSensorType(doc["stype"] | 0);
    if (doc.containsKey("ifxdb")) return saveInfluxDb(doc["ifxdb"] | "", doc["ifxip"] | "", doc["ifxpt"] | 0);
    if (doc.containsKey("pass") && doc.containsKey("ssid")) return saveWifi(doc["ssid"] | "", doc["pass"] | "");
    if (doc.containsKey("ssid")) return saveSSID(doc["ssid"] | "");
    if (doc.containsKey("lat")) return saveGeo(doc["lat"].as<double>(), doc["lon"].as<double>(), doc["geo"] | "");
    if (doc.containsKey("toffset")) return saveTempOffset(doc["toffset"].as<float>());
    if (doc.containsKey("altoffset")) return saveAltitudeOffset(doc["altoffset"].as<float>());
    if (doc.containsKey("sealevel")) return saveSeaLevel(doc["sealevel"].as<float>());
    if (doc.containsKey("hassip")) return saveHassIP(doc["hassip"] | "");
    if (doc.containsKey("hasspt")) return saveHassPort(doc["hasspt"] | 1883);
    if (doc.containsKey("hassusr")) return saveHassUser(doc["hassusr"] | "");
    if (doc.containsKey("hasspsw")) return saveHassPassword(doc["hasspsw"] | "");
    if (doc.containsKey("deepSleep")) return saveDeepSleep(doc["deepSleep"] | 0);
    
    // some actions with chopid validation (for security reasons)
    if (cmd == ((uint16_t)(chipid >> 32)) && act.length() > 0) {
        if (act.equals("wst")) return wifiEnable(doc["wenb"].as<bool>());
        if (act.equals("ist")) return ifxdbEnable(doc["ienb"].as<bool>());
        if (act.equals("dst")) return debugEnable(doc["denb"].as<bool>());
        if (act.equals("i2c")) return saveI2COnly(doc["i2conly"].as<bool>());
        if (act.equals("pst")) return paxEnable(doc["penb"].as<bool>());
        if (act.equals("sse")) return solarEnable(doc["sse"].as<bool>());
        // if (act.equals("cls")) clear();
        // if (act.equals("rbt")) reboot();
        if (act.equals("clb")) performCO2Calibration();
        return true;
    } else {
        Serial.println("[E][CONF] invalid config file!");
        return false;
    }
}

bool getTrackStatusValues(const char *json) {
    StaticJsonDocument<200> doc;
    auto error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("[E][CONF] deserialize Json failed with code "));
        Serial.println(error.c_str());
        return false;
    }
    if (doc.containsKey("spd")) track.spd = doc["spd"] | 0.0;
    if (doc.containsKey("kms")) track.kms = doc["kms"] | 0.0;
    if (doc.containsKey("hrs")) track.hrs = doc["hrs"] | 0;
    if (doc.containsKey("min")) track.min = doc["min"] | 0;
    if (doc.containsKey("seg")) track.seg = doc["seg"] | 0;

    return true;
}


String getDeviceId() {
    uint8_t baseMac[6];
    // Get MAC address for WiFi station
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char baseMacChr[18] = {0};
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]+2);
    return String(baseMacChr);
}

String getAnaireDeviceId() { 
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8) chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    return String(chipId, HEX);
}

String getDeviceIdShort() {
    String devId = getDeviceId();
    devId = devId.substring(13);
    devId.replace(":","");
    return devId;
}

String getStationName() {
    if (geo.isEmpty()) return getAnaireDeviceId();
    String name = ""+geo.substring(0,3);          // GeoHash ~70km https://en.wikipedia.org/wiki/Geohash
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

// void clear() {
//     cfg.clear();
//     Serial.println("-->[CONF] clear settings!");
//     delay(200);
//     reboot();
// }

// void reboot() {
//     Serial.println("-->[CONF] reboot..");
//     delay(100);
//     wd.execute();  // ESP and WiFi reboot
// }

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
