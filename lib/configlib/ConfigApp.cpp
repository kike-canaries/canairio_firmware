#include "ConfigApp.hpp"

void ConfigApp::init(const char app_name[]) {
    _app_name = new char[strlen(app_name) + 1];
    strcpy(_app_name, app_name);
    chipid = ESP.getEfuseMac();
    deviceId = getDeviceId();
    reload();
    // override with debug INFO level (>=3)
    #ifdef CORE_DEBUG_LEVEL
    if (CORE_DEBUG_LEVEL>=3) devmode = true;  
    #endif
    if (devmode) Serial.println("-->[CONF] debug is enable.");
}

void ConfigApp::reload() {
    preferences.begin(_app_name, false);
    // device name or station name
    dname = preferences.getString("dname", "");
    // wifi settings
    wifi_enable = preferences.getBool("wifiEnable", false);
    ssid = preferences.getString("ssid", "");
    pass = preferences.getString("pass", ""); 
    // influx db optional settings
    ifxdb_enable = preferences.getBool("ifxEnable", false);
    ifx.db = preferences.getString("ifxdb", ifx.db);
    ifx.ip = preferences.getString("ifxip", ifx.ip);
    ifx.pt = preferences.getUInt("ifxpt", ifx.pt);
    // canairio api settings
    api_enable = preferences.getBool("apiEnable", false);
    apiusr = preferences.getString("apiusr", "");
    apipss = preferences.getString("apipss", "");
    apisrv = preferences.getString("apisrv", "");
    apiuri = preferences.getString("apiuri", "");
    apiprt = preferences.getInt("apiprt", 80);
    // station and sensor settings
    lat = preferences.getDouble("lat", 0);
    lon = preferences.getDouble("lon", 0);
    alt = preferences.getFloat("alt", 0);
    spd = preferences.getFloat("spd", 0);
    stime = preferences.getInt("stime", 5);
    stype = preferences.getInt("stype", 0);
    toffset = preferences.getFloat("toffset", 0.0);
    devmode = preferences.getBool("debugEnable", false);
    i2conly = preferences.getBool("i2conly", false);

    preferences.end();
}

String ConfigApp::getCurrentConfig() {
    StaticJsonDocument<1000> doc;
    preferences.begin(_app_name, false);
    doc["dname"] = preferences.getString("dname", "");       // device or station name
    doc["stime"] = preferences.getInt("stime", 5);           // sensor measure time
    doc["stype"] = preferences.getInt("stype", 0);          // sensor type { Honeywell, Panasonic, Sensirion };
    doc["wenb"] = preferences.getBool("wifiEnable", false);  // wifi on/off
    doc["ssid"] = preferences.getString("ssid", "");         // influxdb database name
    doc["ienb"] = preferences.getBool("ifxEnable", false);   // ifxdb on/off
    doc["ifxdb"] = preferences.getString("ifxdb", ifx.db);   // influxdb database name
    doc["ifxip"] = preferences.getString("ifxip", ifx.ip);   // influxdb database ip
    doc["ifxpt"] = preferences.getUInt("ifxpt", ifx.pt);     // influxdb sensor tags
    doc["aenb"] = preferences.getBool("apiEnable", false);   // CanAirIO API on/off
    doc["apiusr"] = preferences.getString("apiusr", "");     // API username
    doc["apisrv"] = preferences.getString("apisrv", "");     // API hostname
    doc["apiuri"] = preferences.getString("apiuri", "");     // API uri endpoint
    doc["apiprt"] = preferences.getInt("apiprt", 80);        // API port
    doc["denb"] = preferences.getBool("debugEnable", false); // debug mode enable
    doc["i2conly"] = preferences.getBool("i2conly", false);  // force only i2c sensors
    doc["toffset"] = preferences.getFloat("toffset", 0.0);      // temperature offset
    doc["lskey"] = lastKeySaved;                             // last key saved
    doc["wmac"] = (uint16_t)(chipid >> 32);                  // chipid calculated in init
    doc["wsta"] = wifi_connected;                            // current wifi state 
    doc["vrev"] = REVISION;
    doc["vflv"] = FLAVOR;
    doc["vtag"] = TARGET;
    doc["vmac"] = getDeviceId();
    preferences.end();
    String output;
    serializeJson(doc, output);
    if (devmode) {
        char buf[1000];
        serializeJsonPretty(doc, buf, 1000);
        Serial.printf("-->[CONF] response: %s", buf);
        Serial.println("");
    }
    return output;
}

void ConfigApp::setLastKeySaved(String key){
    lastKeySaved = key;
}

void ConfigApp::saveString(String key, String value){
    preferences.begin(_app_name, false);
    preferences.putString(key.c_str(), value.c_str());
    preferences.end();
    setLastKeySaved(key);
}

void ConfigApp::saveInt(String key, int value){
    preferences.begin(_app_name, false);
    preferences.putInt(key.c_str(), value);
    preferences.end();
    setLastKeySaved(key);
}

void ConfigApp::saveFloat(String key, float value){
    preferences.begin(_app_name, false);
    preferences.putFloat(key.c_str(), value);
    preferences.end();
    setLastKeySaved(key);
}

void ConfigApp::saveBool(String key, bool value){
    preferences.begin(_app_name, false);
    preferences.putBool(key.c_str(), value);
    preferences.end();
    setLastKeySaved(key);
}

bool ConfigApp::saveDeviceName(String name) {
    if (name.length() > 0) {
        saveString("dname",name);
        Serial.println("-->[CONF] set device name to: " + name);
        return true;
    }
    DEBUG("-->[E][CONF] device name is empty!");
    return false;
}

bool ConfigApp::saveSampleTime(int time) {
    if (time >= 5) {
        saveInt("stime", time);
        Serial.print("-->[CONF] sensor sample time set to: ");
        Serial.println(time);
        return true;
    }
    DEBUG("-->[W][CONF] warning: sample time is too low!");
    return false;
}

bool ConfigApp::saveSensorType(int type) {
    saveInt("stype", type);
    Serial.print("-->[CONF] sensor device type: ");
    Serial.println(type);
    return true;
}

int ConfigApp::getSensorType(){
    return stype;
}

bool ConfigApp::saveTempOffset(float offset) {
    saveFloat("toffset", offset);
    Serial.print("-->[CONF] sensor temperature offset: ");
    Serial.println(offset);
    return true;
}

bool ConfigApp::saveWifi(String ssid, String pass){
    if (ssid.length() > 0) {
        preferences.begin(_app_name, false);
        preferences.putString("ssid", ssid);
        preferences.putString("pass", pass);
        preferences.putBool("wifiEnable", true);
        preferences.end();
        setLastKeySaved("ssid");
        wifi_enable = true;
        isNewWifi = true;  // for execute wifi reconnect
        Serial.println("-->[CONF] WiFi credentials saved!");
        log_i("-->[CONF] ssid:%s pass:%s",ssid,pass);
        return true;
    }
    DEBUG("-->[W][CONF] empty Wifi SSID");
    return false;
}

bool ConfigApp::saveInfluxDb(String db, String ip, int pt) {
    if (db.length() > 0 && ip.length() > 0) {
        preferences.begin(_app_name, false);
        preferences.putString("ifxdb", db);
        preferences.putString("ifxip", ip);
        if (pt > 0) preferences.putUInt("ifxpt", pt);
        preferences.putBool("ifxEnable", true);
        preferences.end();
        setLastKeySaved("ifxdb");
        isNewIfxdbConfig = true;
        ifxdb_enable = true;
        Serial.printf("-->[CONF] influxdb: %s@%s:%i\n",db.c_str(),ip.c_str(),pt);
        Serial.println("-->[CONF] influxdb config saved.");
        return true;
    }
    DEBUG("-->[W][CONF] wrong InfluxDb params!");
    return false;
}

bool ConfigApp::saveAPI(String usr, String pass, String srv, String uri, int pt){
    if (usr.length() > 0 && pass.length() > 0 && srv.length() > 0 && uri.length() > 0) {
        preferences.begin(_app_name, false);
        preferences.putString("apiusr", usr);
        preferences.putString("apipss", pass);
        preferences.putString("apisrv", srv);
        preferences.putString("apiuri", uri);
        if (pt > 0) preferences.putInt("apiprt", pt);
        preferences.putBool("apiEnable", true);
        preferences.end();
        setLastKeySaved("api");
        isNewAPIConfig = true;
        api_enable = true;
        log_i("[CONF] API: %s@%s/%s",usr.c_str(),srv.c_str(),uri.c_str());
        Serial.println("-->[CONF] API config saved.");
        return true;
    }
    DEBUG("-->[W][CONF] wrong API params!");
    return false;
}

bool ConfigApp::saveGeo(double lat, double lon, float alt, float spd){
    if (lat != 0 && lon != 0) {
        preferences.begin(_app_name, false);
        preferences.putDouble("lat", lat);
        preferences.putDouble("lon", lon);
        preferences.putFloat("alt", alt);
        preferences.putFloat("spd", spd);
        preferences.end();
        setLastKeySaved("lat");
        log_i("-->[CONF] geo:(%d,%d) alt:%d spd:%d",lat,lon,alt,spd);
        Serial.println("-->[CONF] updated location!");
        return true;
    }
    DEBUG("-->[W][CONF] wrong GEO params!");
    return false;
}

bool ConfigApp::wifiEnable(bool enable) {
    saveBool("wifiEnable", enable);
    wifi_enable = enable;
    Serial.println("-->[CONF] updating WiFi state: " + String(enable));
    return true;
}

bool ConfigApp::ifxdbEnable(bool enable) {
    saveBool("ifxEnable", enable);
    ifxdb_enable = enable;
    Serial.println("-->[CONF] updating InfluxDB state: " + String(enable));
    return true;
}

bool ConfigApp::apiEnable(bool enable) {
    saveBool("apiEnable", enable);
    api_enable = enable;
    Serial.println("-->[CONF] updating API state: " + String(enable));
    return true;
}

bool ConfigApp::debugEnable(bool enable) {
    saveBool("debugEnable", enable);
    devmode = enable;
    Serial.println("-->[CONF] updating debug mode: " + String(enable));
    return true;
}

bool ConfigApp::saveI2COnly(bool enable) {
    saveBool("i2conly", enable);
    i2conly = enable;
    Serial.println("-->[CONF] forced only i2c sensors: " + String(enable));
    return true;
}

bool ConfigApp::save(const char *json) {
    StaticJsonDocument<1000> doc;
    auto error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("-->[E][CONF] deserialize Json failed with code "));
        Serial.println(error.c_str());
        return false;
    }

    if (devmode) {
        char output[1000];
        serializeJsonPretty(doc, output, 1000);
        Serial.printf("-->[CONF] request: %s", output);
        Serial.println("");
    }
    
    uint16_t cmd = doc["cmd"].as<uint16_t>();
    String act = doc["act"] | "";

    if (doc.containsKey("dname")) return saveDeviceName(doc["dname"] | "");
    if (doc.containsKey("stime")) return saveSampleTime(doc["stime"] | 0);
    if (doc.containsKey("stype")) return saveSensorType(doc["stype"] | 0);
    if (doc.containsKey("ifxdb")) return saveInfluxDb(doc["ifxdb"] | "", doc["ifxip"] | "", doc["ifxpt"] | 0);
    if (doc.containsKey("ssid")) return saveWifi(doc["ssid"] | "", doc["pass"] | "");
    if (doc.containsKey("apiusr")) return saveAPI(doc["apiusr"] | "", doc["apipss"] | "", doc["apisrv"] | "", doc["apiuri"] | "", doc["apiprt"] | 0);
    if (doc.containsKey("lat")) return saveGeo(doc["lat"].as<double>(), doc["lon"].as<double>(), doc["alt"].as<float>(), doc["spd"].as<float>());
    if (doc.containsKey("toffset")) return saveTempOffset(doc["toffset"].as<float>());
    

    // some actions with chopid validation (for security reasons)
    if (cmd == ((uint16_t)(chipid >> 32)) && act.length() > 0) {
        if (act.equals("wst")) return wifiEnable(doc["wenb"].as<bool>());
        if (act.equals("ist")) return ifxdbEnable(doc["ienb"].as<bool>());
        if (act.equals("ast")) return apiEnable(doc["aenb"].as<bool>());
        if (act.equals("dst")) return debugEnable(doc["denb"].as<bool>());
        if (act.equals("i2c")) return saveI2COnly(doc["i2conly"].as<bool>());
        if (act.equals("rbt")) reboot();
        if (act.equals("cls")) clear();
        return true;
    } else {
        Serial.println("-->[E][CONF] invalid config file!");
        return false;
    }
}

String ConfigApp::getDeviceId() {
    uint8_t baseMac[6];
    // Get MAC address for WiFi station
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char baseMacChr[18] = {0};
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]+2);
    return String(baseMacChr);
}

bool ConfigApp::isWifiEnable() {
    return wifi_enable;
}

bool ConfigApp::isApiEnable() {
    return api_enable;
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
    preferences.begin(_app_name, false);
    preferences.clear();
    preferences.end();
    Serial.println("-->[CONF] clear settings!");
    reboot();
}

void ConfigApp::reboot() {
    Serial.println("-->[CONF] reboot..");
    delay(100);
    ESP.restart();
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

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
ConfigApp cfg;
#endif
