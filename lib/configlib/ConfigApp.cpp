#include "ConfigApp.hpp"

void ConfigApp::init(const char app_name[]) {
    _app_name = new char[strlen(app_name) + 1];
    strcpy(_app_name, app_name);
    chipid = ESP.getEfuseMac();
    char devId[13];
    sprintf(devId, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
    deviceId = new char[strlen(devId) + 1];
    strcpy(deviceId, devId);
    reload();
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
    ifx.db = preferences.getString("ifxdb", "");
    ifx.ip = preferences.getString("ifxip", "");
    ifx.pt = preferences.getUInt("ifxpt", 8086);
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
    stype = preferences.getInt("stype", -1);

    preferences.end();
}

String ConfigApp::getCurrentConfig() {
    StaticJsonDocument<500> doc;
    preferences.begin(_app_name, false);
    doc["dname"] = preferences.getString("dname", "");       // device or station name
    doc["wenb"] = preferences.getBool("wifiEnable", false);  // wifi on/off
    doc["ssid"] = preferences.getString("ssid", "");         // influxdb database name
    doc["ienb"] = preferences.getBool("ifxEnable", false);   // ifxdb on/off
    doc["ifxdb"] = preferences.getString("ifxdb", "");       // influxdb database name
    doc["ifxip"] = preferences.getString("ifxip", "");       // influxdb database ip
    doc["ifusr"] = preferences.getString("ifusr", "");       // influxdb sensorid name
    doc["ifxpt"] = preferences.getUInt("ifxpt", 8086);       // influxdb sensor tags
    doc["stime"] = preferences.getInt("stime", 5);           // sensor measure time
    doc["aenb"] = preferences.getBool("apiEnable", false);   // CanAirIO API on/off
    doc["apiusr"] = preferences.getString("apiusr", "");     // API username
    doc["apisrv"] = preferences.getString("apisrv", "");     // API hostname
    doc["apiuri"] = preferences.getString("apiuri", "");     // API uri endpoint
    doc["apiprt"] = preferences.getInt("apiprt", 80);        // API port
    doc["stype"] = preferences.getInt("stype", -1);          // sensor type { Honeywell, Panasonic, Sensirion };
    doc["wmac"] = (uint16_t)(chipid >> 32);
    preferences.end();
    String output;
    serializeJson(doc, output);
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

void ConfigApp::saveBool(String key, bool value){
    preferences.begin(_app_name, false);
    preferences.putBool(key.c_str(), value);
    preferences.end();
    setLastKeySaved(key);
}

bool ConfigApp::saveDeviceName(String name) {
    if (name.length() > 0) {
        saveString("dname",name);
        Serial.println("-->[CONFIG] set device name to: " + name);
        return true;
    }
    return false;
}

bool ConfigApp::saveSampleTime(int time) {
    if (time >= 5) {
        saveInt("stime", time);
        Serial.print("-->[CONFIG] sensor sample time set to: ");
        Serial.println(time);
        return true;
    }
    return false;
}

bool ConfigApp::saveSensorType(int type) {
    saveInt("stype", type);
    Serial.print("-->[CONFIG] sensor device type: ");
    Serial.println(type);
    return true;
}

bool ConfigApp::saveWifi(String ssid, String pass){
    if (ssid.length() > 0 && pass.length() > 0) {
        preferences.begin(_app_name, false);
        preferences.putString("ssid", ssid);
        preferences.putString("pass", pass);
        preferences.putBool("wifiEnable", true);
        preferences.end();
        setLastKeySaved("wifi");
        wifi_enable = true;
        isNewWifi = true;  // for execute wifi reconnect
        Serial.println("-->[CONFIG] WiFi credentials saved!");
        log_i("-->[CONFIG] ssid:%s pass:%s",ssid,pass);
        return true;
    }
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
        Serial.println("-->[CONFIG] influxdb saved: db:"+db+" ip:"+ip);
        log_i("-->[CONFIG] %s",getCurrentConfig());
        return true;
    }
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
        Serial.println("-->[CONFIG] API credentials saved!");
        log_i("-->[CONFIG] usr:%s srv:%s uri:%s",usr,srv,uri);
        return true;
    }
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
        setLastKeySaved("geo");
        Serial.println("-->[CONFIG] updated location!");
        log_i("-->[CONFIG] geo:(%d,%d) alt:%d spd:%d",lat,lng,alt,spd);
        return true;
    }
    return false;
}

bool ConfigApp::wifiEnable(bool enable) {
    saveBool("wifiEnable", enable);
    wifi_enable = enable;
    Serial.println("-->[CONFIG] Updating WiFi state: " + String(enable));
    return true;
}

bool ConfigApp::ifxdbEnable(bool enable) {
    saveBool("ifxEnable", enable);
    ifxdb_enable = enable;
    Serial.println("-->[CONFIG] Updating InfluxDB state: " + String(enable));
    return true;
}

bool ConfigApp::apiEnable(bool enable) {
    saveBool("apiEnable", enable);
    api_enable = enable;
    Serial.println("-->[CONFIG] Updating API state: " + String(enable));
    return true;
}

bool ConfigApp::save(const char *json) {
    StaticJsonDocument<1000> doc;
    auto error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("-->[E][CONFIG] deserialize Json failed with code "));
        Serial.println(error.c_str());
        return false;
    }
    // char* output[1000];
    // serializeJsonPretty(doc, output, 1000);
    // log_n("[CONFIG] JSON: %s",output);

    String key = doc["key"] | "";
    uint16_t cmd = doc["cmd"].as<uint16_t>();
    String act = doc["act"] | "";

    if (key.length()>0) {
        if (key.equals("dname")) return saveDeviceName(doc["dname"] | "");
        if (key.equals("stime")) return saveSampleTime(doc["stime"] | 0);
        if (key.equals("stype")) return saveSensorType(doc["stype"] | -1);
        if (key.equals("ifxdb")) return saveInfluxDb(doc["ifxdb"]|"",doc["ifxip"]|"",doc["ifxpt"]|0);
        if (key.equals("wifi")) return saveWifi(doc["ssid"]|"",doc["pass"]|"");
        if (key.equals("api")) return saveAPI(doc["apiusr"]|"",doc["apipss"]|"",doc["apisrv"]|"",doc["apiuri"]|"",doc["apiprt"]|0);
        if (key.equals("geo")) return saveGeo(doc["lat"].as<double>(),doc["lon"].as<double>(),doc["alt"].as<float>(),doc["spd"].as<float>());
        if (key.equals("wst")) return wifiEnable(doc["wenb"].as<bool>()); 
        if (key.equals("ist")) return ifxdbEnable(doc["ienb"].as<bool>());
        if (key.equals("ast")) return apiEnable(doc["aenb"].as<bool>());
    }
    // String tdname = doc["dname"] | "";
    // String tifxdb = doc["ifxdb"] | "";
    // String tifxip = doc["ifxip"] | "";
    // uint16_t tifxpt = doc["ifxpt"].as<uint16_t>();
    // String tssid = doc["ssid"] | "";
    // String tpass = doc["pass"] | "";
    // String tapiusr = doc["apiusr"] | "";
    // String tapipss = doc["apipss"] | "";
    // String tapisrv = doc["apisrv"] | "";
    // String tapiuri = doc["apiuri"] | "";
    // int tapiprt = doc["apiprt"] | 80;
    // int tstime = doc["stime"] | 0;
    // double tlat = doc["lat"].as<double>();
    // double tlon = doc["lon"].as<double>();
    // float talt = doc["alt"].as<float>();
    // float tspd = doc["spd"].as<float>();
    // bool wenb = doc["wenb"].as<bool>();
    // bool ienb = doc["ienb"].as<bool>();
    // bool aenb = doc["aenb"].as<bool>();

    else if (cmd == ((uint16_t)(chipid >> 32)) && act.length() > 0) {
        // reboot command
        if (act.equals("rbt")) {
            Serial.println("-->[CONFIG] reboot..");
            reboot();
        }
        // clear preferences command
        if (act.equals("cls")) {
            preferences.begin(_app_name, false);
            preferences.clear();
            preferences.end();
            reboot();
        }
        
    } else {
        Serial.println("-->[E][CONFIG] invalid config file!");
        return false;
    }
    return true;
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

void ConfigApp::reboot() {
    delay(100);
    ESP.restart();
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
ConfigApp cfg;
#endif
