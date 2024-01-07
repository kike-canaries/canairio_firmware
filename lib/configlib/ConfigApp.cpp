#include "ConfigApp.hpp"

#define X(kname, kreal, ktype) kreal, 
char const *keys[] = { CONFIG_KEYS_LIST };
#undef X

#define X(kname, kreal, ktype) ktype, 
int keys_type[] = { CONFIG_KEYS_LIST };
#undef X

void ConfigApp::init(const char app_name[]) {
    _app_name = new char[strlen(app_name) + 1];
    strcpy(_app_name, app_name);
    chipid = ESP.getEfuseMac();
    deviceId = getDeviceId();
    reload();
    // override with debug INFO level (>=3)
#if CORE_DEBUG_LEVEL >= 1
    devmode = true;
#endif
    if (devmode) Serial.println("-->[CONF] debug is enable.");
}

void ConfigApp::reload() {
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RO_MODE);
    // device name or station name
    dname = preferences.getString("dname", "");
    // wifi settings
    wifi_enable = preferences.getBool(getKey(CONFKEYS::KBWIFIEN).c_str(), false);
    ssid = preferences.getString("ssid", "");
    pass = preferences.getString("pass", ""); 
    // influx db optional settings
    ifxdb_enable = preferences.getBool(getKey(CONFKEYS::KBIFXENB).c_str(), false);
    ifx.db = preferences.getString("ifxdb", ifx.db);
    ifx.ip = preferences.getString("ifxip", ifx.ip);
    ifx.pt = preferences.getInt("ifxpt", ifx.pt);
    // station and sensor settings
    lat = preferences.getDouble("lat", 0);
    lon = preferences.getDouble("lon", 0);
    geo = preferences.getString("geo", "");
    stime = preferences.getInt("stime", 5);
    stype = preferences.getInt("stype", 0);
    sTX = preferences.getInt("sTX", -1);
    sRX = preferences.getInt("sRX", -1);
    toffset = preferences.getFloat("toffset", 0.0);
    altoffset = preferences.getFloat("altoffset", 0.0);
    sealevel = preferences.getFloat("sealevel", 1013.25);
    devmode = preferences.getBool("debugEnable", false);
    pax_enable = preferences.getBool(getKey(CONFKEYS::KBPAXENB).c_str(), true);
    i2conly = preferences.getBool("i2conly", false);
    solarmode = preferences.getBool("solarEnable", false);
    deepSleep = preferences.getInt("deepSleep", 0);
    hassip = preferences.getString("hassip", "");
    hasspt = preferences.getInt("hasspt", 1883);
    hassusr = preferences.getString("hassusr", "");
    hasspsw = preferences.getString("hasspsw", "");

    preferences.end();
}

String ConfigApp::getCurrentConfig() {
    StaticJsonDocument<1000> doc;
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RO_MODE);
    doc["dname"] = preferences.getString("dname", "");       // device or station name
    doc["stime"] = preferences.getInt("stime", 5);           // sensor measure time
    doc["stype"] = preferences.getInt("stype", 0);           // sensor UART type;
    doc["sRX"] = preferences.getInt("sRX", -1);           // sensor UART type;
    doc["sTX"] = preferences.getInt("sTX", -1);           // sensor UART type;
    doc["wenb"] = preferences.getBool(getKey(CONFKEYS::KBWIFIEN).c_str(), false);  // wifi on/off
    doc["ssid"] = preferences.getString("ssid", "");         // influxdb database name
    doc["ienb"] = preferences.getBool(getKey(CONFKEYS::KBIFXENB).c_str(), false);   // ifxdb on/off
    doc["ifxdb"] = preferences.getString("ifxdb", ifx.db);   // influxdb database name
    doc["ifxip"] = preferences.getString("ifxip", ifx.ip);   // influxdb database ip
    doc["ifxpt"] = preferences.getInt("ifxpt", ifx.pt);     // influxdb sensor tags
    doc["geo"] = preferences.getString("geo", "");           // influxdb GeoHash tag
    doc["denb"] = preferences.getBool("debugEnable", false); // debug mode enable
    doc["penb"] = preferences.getBool(getKey(CONFKEYS::KBPAXENB).c_str(), true);    // PaxCounter enable
    doc["i2conly"] = preferences.getBool("i2conly", false);  // force only i2c sensors
    doc["sse"] = preferences.getBool("solarEnable", false);  // Enable solar station
    doc["deepSleep"] = preferences.getInt("deepSleep", 0);  // deep sleep time in seconds
    doc["toffset"] = preferences.getFloat("toffset", 0.0);   // temperature offset
    doc["altoffset"] = preferences.getFloat("altoffset",0.0);// altitude offset
    doc["sealevel"] = preferences.getFloat("sealevel",1013.25);// altitude offset
    doc["hassip"] = preferences.getString("hassip", "");     // Home Assistant MQTT server ip
    doc["hasspt"] = preferences.getInt("hasspt", 1883);      // Home Assistant MQTT server port
    doc["hassusr"] = preferences.getString("hassusr", "");   // Home Assistant MQTT user
    // doc["hasspsw"] = preferences.getString("hasspsw", "");// Home Assistant MQTT password
    doc["lskey"] = lastKeySaved;                             // last key saved
    doc["wmac"] = (uint16_t)(chipid >> 32);                  // chipid calculated in init
    doc["anaireid"] =  getStationName();                     // deviceId for Anaire cloud
    doc["wsta"] = wifi_connected;                            // current wifi state 
    doc["vrev"] = REVISION;
    doc["vflv"] = FLAVOR;
    doc["vtag"] = TARGET;
    doc["vmac"] = getDeviceId();
    preferences.end();
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

void ConfigApp::setLastKeySaved(String key){
    lastKeySaved = key;
}

void ConfigApp::saveString(String key, String value){
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RW_MODE);
    preferences.putString(key.c_str(), value.c_str());
    preferences.end();
    setLastKeySaved(key);
}

void ConfigApp::saveString(CONFKEYS key, String value){
    saveString(getKey(key),value);
}

String ConfigApp::getString(String key, String defaultValue){
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RO_MODE);
    String out = preferences.getString(key.c_str(), defaultValue);
    preferences.end();
    return out;
}

String ConfigApp::getString(CONFKEYS key, String defaultValue){
    return getString(getKey(key),defaultValue);
}

void ConfigApp::saveInt(String key, int value){
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RW_MODE);
    preferences.putInt(key.c_str(), value);
    preferences.end();
    setLastKeySaved(key);
}

void ConfigApp::saveInt(CONFKEYS key, int value){
    saveInt(getKey(key),value);
}

int32_t ConfigApp::getInt(String key, int defaultValue){
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RO_MODE);
    int32_t out = preferences.getInt(key.c_str(), defaultValue);
    preferences.end();
    return out;
}

int32_t ConfigApp::getInt(CONFKEYS key, int defaultValue){ 
    return getInt(getKey(key),defaultValue);
}

void ConfigApp::saveBool(String key, bool value){
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RW_MODE);
    preferences.putBool(key.c_str(), value);
    preferences.end();
    setLastKeySaved(key);
}

void ConfigApp::saveBool(CONFKEYS key, bool value){
    saveBool(getKey(key),value);
}

bool ConfigApp::getBool(String key, bool defaultValue){
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RO_MODE);
    bool out = preferences.getBool(key.c_str(), defaultValue);
    preferences.end();
    return out;
}

bool ConfigApp::getBool(CONFKEYS key, bool defaultValue){
    return getBool(getKey(key),defaultValue);
}

void ConfigApp::saveFloat(String key, float value){
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RW_MODE);
    preferences.putFloat(key.c_str(), value);
    preferences.end();
    setLastKeySaved(key);
}

void ConfigApp::saveFloat(CONFKEYS key, float value){
    saveFloat(getKey(key),value);
}

float ConfigApp::getFloat(String key, float defaultValue){
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RO_MODE);
    float out = preferences.getFloat(key.c_str(), defaultValue);
    preferences.end();
    return out;
}

float ConfigApp::getFloat(CONFKEYS key, float defaultValue){
    return getFloat(getKey(key),defaultValue);
}

PreferenceType ConfigApp::keyType(String key) {
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RO_MODE);
    PreferenceType type = preferences.getType(key.c_str());
    preferences.end();
    return type;
}



bool ConfigApp::isKey(String key) {
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RO_MODE);
    bool iskey = preferences.isKey(key.c_str());
    preferences.end();
    return iskey;
}

bool ConfigApp::isKey(CONFKEYS key) {
    return isKey(getKey(key));
}

String ConfigApp::getKey(CONFKEYS key) {
  if (key < 0 || key > CONFKEYS::KCOUNT) return "";
  return String(keys[key]);
}

ConfKeyType ConfigApp::getKeyType(CONFKEYS key) {
  if (key < 0 || key > CONFKEYS::KCOUNT) return ConfKeyType::UNKNOWN;
  return (ConfKeyType)keys_type[key];
}

ConfKeyType ConfigApp::getKeyType(String key) {
  for (int i = 0; i < KCOUNT; i++) {
    if (key.equals(keys[i])) return (ConfKeyType)keys_type[i];
  }
  return ConfKeyType::UNKNOWN;
}

/**
 * @brief DEPRECATED
 */ 
bool ConfigApp::saveDeviceName(String name) {
    if (name.length() > 0) {
        saveString("dname",name);
        Serial.println("-->[CONF] set device name to: " + name);
        return true;
    }
    DEBUG("[E][CONF] device name is empty!");
    return false;
}

bool ConfigApp::saveSampleTime(int time) {
    if (time >= 5) {
        saveInt("stime", time);
        stime=time;
        Serial.printf("-->[CONF] set sample time to\t: %d\r\n", time);
        return true;
    }
    DEBUG("[W][CONF] warning: sample time is too low!");
    return false;
}

/**
 * @brief ConfigApp::saveSensorType
 * @param type UART sensor type. Sync it with Android app
 * @return true (compatibility)
 */
bool ConfigApp::saveSensorType(int type) {
    saveInt("stype", type);
    Serial.printf("-->[CONF] sensor device type\t: %d\r\n", type);
    return true;
}

/**
 * @brief ConfigApp::saveSensorPins
 * @param tx UART sensor TX
 * @param rx UART sensor RX
 * @return true (compatibility)
 */
bool ConfigApp::saveSensorPins(int tx, int rx) {
    saveInt("sTX", tx);
    saveInt("sRX", rx);
    Serial.printf("-->[CONF] sensor UART TX/RX\t: %d/%d\r\n", tx, rx);
    return true;
}

int ConfigApp::getSensorType(){
    return stype;
}

/**
 * @brief ConfigApp::saveWifiEnable
 * @param unit save the sensor UNIT selected
 * @return 
 */
bool ConfigApp::saveUnitSelected(int unit){
    saveInt("unit", unit);
    Serial.printf("-->[CONF] default unit to \t: %d\r\n", unit);
    return true;
}

/**
 * @brief ConfigApp::getUnitSelected
 * @return unit selected and saved by user (default PM2.5)
 */
int ConfigApp::getUnitSelected(){
    return getInt("unit", 2); 
}

bool ConfigApp::saveTempOffset(float offset) {
    saveFloat("toffset", offset);
    Serial.printf("-->[CONF] sensor temp offset\t: %0.2f\r\n", offset);
    return true;
}

bool ConfigApp::saveAltitudeOffset(float offset) {
    saveFloat("altoffset", offset);
    Serial.printf("-->[CONF] sensor altitude offset\t: %0.2f\r\n", offset);
    if(mRemoteConfigCallBacks!=nullptr) this->mRemoteConfigCallBacks->onAltitudeOffset(offset);
    return true;
}

bool ConfigApp::saveSeaLevel(float hpa) {
    saveFloat("sealevel", hpa);
    Serial.printf("-->[CONF] sea level pressure\t: %0.2f\r\n", hpa);
    if(mRemoteConfigCallBacks!=nullptr) this->mRemoteConfigCallBacks->onSeaLevelPressure(hpa);
    return true;
}

bool ConfigApp::saveSSID(String ssid){
    if (ssid.length() > 0) {
        std::lock_guard<std::mutex> lck(config_mtx);
        preferences.begin(_app_name, RW_MODE);
        preferences.putString("ssid", ssid);
        preferences.end();
        setLastKeySaved("ssid");
        Serial.println("-->[CONF] WiFi SSID saved!");
        return true;
    }
    DEBUG("[W][CONF] empty Wifi SSID");
    return false;
}

bool ConfigApp::saveWifi(String ssid, String pass){
    if (ssid.length() > 0) {
        std::lock_guard<std::mutex> lck(config_mtx);
        preferences.begin(_app_name, RW_MODE);
        preferences.putString("ssid", ssid);
        preferences.putString("pass", pass);
        preferences.putBool(getKey(CONFKEYS::KBWIFIEN).c_str(), true);
        preferences.end();
        setLastKeySaved("ssid");
        wifi_enable = true;
        isNewWifi = true;  // for execute wifi reconnect
        Serial.println("-->[CONF] WiFi credentials saved!");
        log_i("[CONF] ssid:%s pass:%s",ssid,pass);
        return true;
    }
    DEBUG("[W][CONF] empty Wifi SSID");
    return false;
}

bool ConfigApp::saveInfluxDb(String db, String ip, int pt) {
    if (db.length() > 0 && ip.length() > 0) {
        std::lock_guard<std::mutex> lck(config_mtx);
        preferences.begin(_app_name, RW_MODE);
        preferences.putString("ifxdb", db);
        preferences.putString("ifxip", ip);
        if (pt > 0) preferences.putInt("ifxpt", pt);
        preferences.putBool(getKey(CONFKEYS::KBIFXENB).c_str(), true);
        preferences.end();
        setLastKeySaved("ifxdb");
        ifxdb_enable = true;
        Serial.printf("-->[CONF] influxdb: %s@%s:%i\r\n",db.c_str(),ip.c_str(),pt);
        Serial.println("-->[CONF] influxdb config saved.");
        return true;
    }
    DEBUG("[W][CONF] wrong InfluxDb params!");
    return false;
}

bool ConfigApp::saveGeo(double lat, double lon, String geo){
    if (lat != 0 && lon != 0) {
        std::lock_guard<std::mutex> lck(config_mtx);
        preferences.begin(_app_name, RW_MODE);
        preferences.putDouble("lat", lat);
        preferences.putDouble("lon", lon);
        preferences.putString("geo", geo);
        preferences.end();
        this->lat = lat;
        this->lon = lon;
        this->geo = geo;
        setLastKeySaved("geo");
        Serial.printf("-->[CONF] New Geohash: %s \t: (%.4f,%.4f)\r\n",geo,lat,lon);
        return true;
    }
    DEBUG("[W][CONF] wrong GEO params!");
    return false;
}

bool ConfigApp::saveGeo(String geo){
    if (geo.length() > 5) {
        float lat;
        float lon;
        geohash.decode(geo.c_str(),geo.length(),&lon,&lat);
        log_i("[CONF] Geohash decoder: %s (%.4f,%.4f)\r\n",geo,lat,lon);
        cfg.saveGeo(lat,lon, geo);
        setLastKeySaved("geo");
        return true;
    }
    DEBUG("[W][CONF] wrong GEO params!");
    return false;
}

bool ConfigApp::wifiEnable(bool enable) {
    saveBool(CONFKEYS::KBWIFIEN, enable);
    wifi_enable = enable;
    Serial.println("-->[CONF] updating WiFi state\t: " + String(enable));
    return true;
}

bool ConfigApp::ifxdbEnable(bool enable) {
    saveBool(CONFKEYS::KBIFXENB, enable);
    ifxdb_enable = enable;
    Serial.println("-->[CONF] updating InfluxDB state\t: " + String(enable));
    return true;
}

bool ConfigApp::debugEnable(bool enable) {
    saveBool(CONFKEYS::KDEBUG, enable);
    devmode = enable;
    Serial.println("-->[CONF] new debug mode\t: " + String(enable));
    return true;
}

bool ConfigApp::paxEnable(bool enable) {
    saveBool(CONFKEYS::KBPAXENB, enable);
    pax_enable = enable;
    Serial.println("-->[CONF] new PaxCounter mode\t: " + String(enable));
    return true;
}

bool ConfigApp::solarEnable(bool enable) {
    saveBool(CONFKEYS::KBSOLARE, enable);
    solarmode = enable;
    Serial.println("-->[CONF] Solar Station mode\t: " + String(enable));
    return true;
}

bool ConfigApp::saveDeepSleep(int seconds){
    saveInt(CONFKEYS::KIDEEPSL, seconds);
    deepSleep = seconds;
    Serial.printf("-->[CONF] deep sleep time to\t: %d\r\n", seconds);
    return true;
}

bool ConfigApp::saveI2COnly(bool enable) {
    saveBool(CONFKEYS::KBI2COLY, enable);
    i2conly = enable;
    Serial.println("-->[CONF] forced only i2c sensors\t: " + String(enable));
    return true;
}

bool ConfigApp::saveHassIP(String ip) {
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RW_MODE);
    preferences.putString("hassip", ip);
    preferences.end();
    setLastKeySaved("hassip");
    Serial.printf("-->[CONF] Hass local IP \t: %s saved.\r\n",ip.c_str());
    return true;
}

bool ConfigApp::saveHassPort(int port) {
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RW_MODE);
    preferences.putInt("hasspt", port);
    preferences.end();
    setLastKeySaved("hasspt");
    Serial.printf("-->[CONF] Hass Port  \t: %i saved.\r\n", port);
    return true;
}

bool ConfigApp::saveHassUser(String user) {
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RW_MODE);
    preferences.putString("hassusr", user);
    preferences.end();
    setLastKeySaved("hassusr");
    Serial.printf("-->[CONF] Hass User  \t: %s saved.\r\n", user.c_str());
    return true;
}

bool ConfigApp::saveHassPassword(String passw) {
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RW_MODE);
    preferences.putString("hasspsw", passw);
    preferences.end();
    setLastKeySaved("hasspsw");
    if (devmode) Serial.printf("-->[CONF] Hass password %s saved.\r\n", passw.c_str());
    else Serial.println("-->[CONF] Hass password saved.");
    return true;
}

bool ConfigApp::save(const char *json) {
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

    if (doc.containsKey("dname")) return saveDeviceName(doc["dname"] | "");
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
        if (act.equals("cls")) clear();
        if (act.equals("rbt")) reboot();
        if (act.equals("clb")) performCO2Calibration();
        return true;
    } else {
        Serial.println("[E][CONF] invalid config file!");
        return false;
    }
}

bool ConfigApp::getTrackStatusValues(const char *json) {
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


String ConfigApp::getDeviceId() {
    uint8_t baseMac[6];
    // Get MAC address for WiFi station
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char baseMacChr[18] = {0};
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]+2);
    return String(baseMacChr);
}

String ConfigApp::getAnaireDeviceId() { 
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8) chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    return String(chipId, HEX);
}

String ConfigApp::getDeviceIdShort() {
    String devId = getDeviceId();
    devId = devId.substring(13);
    devId.replace(":","");
    return devId;
}

String ConfigApp::getStationName() {
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

String ConfigApp::getVersion() {
    return "v"+String(VERSION)+"r"+String(REVISION)+String(TARGET);
}

bool ConfigApp::isWifiEnable() {
    return wifi_enable;
}

bool ConfigApp::isPaxEnable() {
    return pax_enable;
}

bool ConfigApp::isIfxEnable() {
    return ifxdb_enable;
}

void ConfigApp::setWifiConnected(bool connected){
    wifi_connected = connected;
}

bool ConfigApp::isWifiConnected() {
    return wifi_connected;
}

void ConfigApp::clear() {
    std::lock_guard<std::mutex> lck(config_mtx);
    preferences.begin(_app_name, RW_MODE);
    preferences.clear();
    preferences.end();
    Serial.println("-->[CONF] clear settings!");
    delay(200);
    reboot();
}

void ConfigApp::reboot() {
    Serial.println("-->[CONF] reboot..");
    delay(100);
    wd.execute();  // ESP and WiFi reboot
}

void ConfigApp::performCO2Calibration() {
    if(mRemoteConfigCallBacks!=nullptr) this->mRemoteConfigCallBacks->onCO2Calibration();
}

void ConfigApp::saveBrightness(int value){
    saveInt("bright",value);
}

int32_t ConfigApp::getBrightness(){
    return getInt("bright",30);
}

void ConfigApp::colorsInvertedEnable(bool enable){
    saveBool("cinverted",enable);
}

void ConfigApp::DEBUG(const char *text, const char *textb) {
    if (devmode) {
        _debugPort.print(text);
        if (textb) {
            _debugPort.print(" ");
            _debugPort.print(textb);
        }
        _debugPort.println();
    }
}

void ConfigApp::setRemoteConfigCallbacks(RemoteConfigCallbacks* pCallbacks){
    mRemoteConfigCallBacks = pCallbacks;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
ConfigApp cfg;
#endif
