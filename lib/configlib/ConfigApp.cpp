#include "ConfigApp.hpp"

void ConfigApp::init(const char app_name[]){
  _app_name = new char[strlen(app_name)+1];
  strcpy(_app_name,app_name); 
  chipid=ESP.getEfuseMac();
  char devId[13];
  sprintf(devId,"%04X%08X",(uint16_t)(chipid >> 32),(uint32_t)chipid);
  deviceId = new char[strlen(devId)+1];
  strcpy(deviceId,devId);
  reload();
}

void ConfigApp::reload(){
  preferences.begin(_app_name,false);
  // device name or station name
  dname = preferences.getString("dname","");
  // wifi settings
  wifiEnable = preferences.getBool("wifiEnable",false);
  ssid = preferences.getString("ssid","");
  pass = preferences.getString("pass","");
  // influx db optional settings
  ifxEnable = preferences.getBool("ifxEnable",false);
  ifxdb = preferences.getString("ifxdb","");
  ifxip = preferences.getString("ifxip","");
  ifxpt = preferences.getUInt("ifxpt",8086);
  // ifxtg = preferences.getString("ifxtg","");
  ifusr = preferences.getString("ifusr","");
  ifpss = preferences.getString("ifpss","");
  // canairio api settings
  apiEnable = preferences.getBool("apiEnable",false);
  apiusr = preferences.getString("apiusr","");
  apipss = preferences.getString("apipss","");
  apisrv = preferences.getString("apisrv","");
  apiuri = preferences.getString("apiuri","");
  apiprt = preferences.getInt("apiprt",80);
  // station and sensor settings
  lat   = preferences.getDouble("lat",0);
  lon   = preferences.getDouble("lon",0);
  alt = preferences.getFloat("alt",0);
  spd = preferences.getFloat("spd",0);
  stime = preferences.getInt("stime",5);

  preferences.end();
}

String ConfigApp::getCurrentConfig(){
  StaticJsonDocument<500> doc;
  preferences.begin(_app_name,false);
  doc["dname"]  =  preferences.getString("dname","");       // device or station name
  doc["wenb"]   =  preferences.getBool("wifiEnable",false); // wifi on/off
  doc["ssid"]   =  preferences.getString("ssid","");        // influxdb database name
  doc["ienb"]   =  preferences.getBool("ifxEnable",false);  // ifxdb on/off
  doc["ifxdb"]  =  preferences.getString("ifxdb","");       // influxdb database name
  doc["ifxip"]  =  preferences.getString("ifxip","");       // influxdb database ip
  doc["ifusr"]  =  preferences.getString("ifusr", "");      // influxdb sensorid name
  doc["ifxpt"]  =  preferences.getUInt("ifxpt",8086);       // influxdb sensor tags
  doc["stime"]  =  preferences.getInt("stime",5);           // sensor measure time
  doc["aenb"]   =  preferences.getBool("apiEnable",false);  // CanAirIO API on/off
  doc["apiusr"] =  preferences.getString("apiusr","");      // API username
  doc["apisrv"] =  preferences.getString("apisrv","");      // API hostname
  doc["apiuri"] =  preferences.getString("apiuri","");      // API uri endpoint
  doc["apiprt"] =  preferences.getInt("apiprt",80);         // API port
  doc["wmac"]   =  (uint16_t)(chipid >> 32);
  preferences.end();
  String output;
  serializeJson(doc,output);
  return output;
}

bool ConfigApp::save(const char *json){
  StaticJsonDocument<1000> doc;
  auto error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("-->[E][CONFIG] deserialize Json failed with code "));
    Serial.println(error.c_str());

    // setErrorCode(ecode_json_parser_error);
    return false;
  }
  String tdname = doc["dname"] | "";
  String tifxdb = doc["ifxdb"] | "";
  String tifxip = doc["ifxip"] | "";
  String tifusr = doc["ifusr"] | "";
  String tifpss = doc["ifpss"] | "";
  String tifcer = doc["ifcer"] | "";
  String tifxtg = doc["ifxtg"] | "";
  String tssid  = doc["ssid"]  | "";
  String tpass  = doc["pass"]  | "";
  String tapiusr= doc["apiusr"]| "";
  String tapipss= doc["apipss"]| "";
  String tapisrv= doc["apisrv"]| "";
  String tapiuri= doc["apiuri"]| "";
  int tapiprt   = doc["apiprt"] | 80;
  int tstime    = doc["stime"] | 0;
  double tlat   = doc["lat"].as<double>();
  double tlon   = doc["lon"].as<double>();
  float talt    = doc["alt"].as<float>();
  float tspd    = doc["spd"].as<float>();
  uint16_t cmd  = doc["cmd"].as<uint16_t>();
  uint16_t tifxpt = doc["ifxpt"].as<uint16_t>();
  String act    = doc["act"]  | "";
  bool wenb     = doc["wenb"].as<bool>();
  bool ienb     = doc["ienb"].as<bool>();
  bool aenb     = doc["aenb"].as<bool>();

  if (tdname.length()>0) {
    preferences.begin(_app_name, false);
    preferences.putString("dname", tdname );
    preferences.end();
    Serial.println("-->[CONFIG] set device name to"+tdname);
  }
  else if (tifxdb.length()>0 && tifxip.length()>0) {
    preferences.begin(_app_name, false);
    preferences.putString("ifxdb", tifxdb );
    preferences.putString("ifxip", tifxip );
    if (tifxpt > 0) {
      preferences.putUInt("ifxpt", tifxpt );
    }
    if (tifusr.length() > 0 && tifpss.length() > 0) {
      preferences.putString("ifusr", tifusr);
      preferences.putString("ifpss", tifpss);
    }
    if (tifcer.length() > 0) {
      preferences.putString("ifcer", tifcer );
    }
    preferences.putBool("ifxEnable",true);
    preferences.end();
    isNewIfxdbConfig=true;
    ifxEnable = true;
    Serial.println("-->[CONFIG] influxdb config saved!");
    Serial.print("-->[CONFIG] ");
    Serial.println(getCurrentConfig());
  }
  else if (tssid.length()>0 && tpass.length()>0){
    preferences.begin(_app_name, false);
    preferences.putString("ssid", tssid);
    preferences.putString("pass", tpass);
    preferences.putBool("wifiEnable",true);
    preferences.end();
    wifiEnable=true;
    isNewWifi=true;  // for execute wifi reconnect
    Serial.println("-->[CONFIG] WiFi credentials saved!");
  }
  else if (tapiusr.length()>0 && tapipss.length()>0 && tapisrv.length()>0 && tapiuri.length()>0){
    preferences.begin(_app_name, false);
    preferences.putString("apiusr", tapiusr);
    preferences.putString("apipss", tapipss);
    preferences.putString("apisrv", tapisrv);
    preferences.putString("apiuri", tapiuri);
    preferences.putInt("apiprt", tapiprt);
    preferences.putBool("apiEnable",true);
    preferences.end();
    isNewAPIConfig = true;
    apiEnable = true;
    Serial.println("-->[CONFIG] API credentials saved!");
  }
  else if (tlat != 0 && tlon != 0) {
    preferences.begin(_app_name, false);
    preferences.putDouble("lat",tlat);
    preferences.putDouble("lon",tlon);
    preferences.putFloat("alt",talt);
    preferences.putFloat("spd",tspd);
    preferences.end();
    Serial.print("-->[CONFIG] updated location to: ");
    Serial.print(tlat); Serial.print(","); Serial.println(tlon);
    Serial.print("-->[CONFIG] altitude: "); Serial.println(talt);
    Serial.print("-->[CONFIG] speed: "); Serial.println(tspd);
  }
  else if (tstime>=5) {
    preferences.begin(_app_name, false);
    preferences.putInt("stime", tstime);
    preferences.end();
    Serial.println("-->[CONFIG] sensor sample time saved!");
  }
  else if (cmd==((uint16_t)(chipid >> 32)) && act.length()>0){
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
    // enable/disable wifi
    if (act.equals("wst")) {
      preferences.begin(_app_name, false);
      preferences.putBool("wifiEnable", wenb);
      preferences.end();
      wifiEnable = wenb;
      Serial.println("-->[CONFIG] Updating WiFi state: "+wenb);
    }
    // enable/disable influxDb state
    if (act.equals("ist")) {
      preferences.begin(_app_name, false);
      preferences.putBool("ifxEnable", ienb);
      preferences.end();
      ifxEnable = ienb;
      Serial.println("-->[CONFIG] Updating InfluxDB state: "+ienb);
    }
    // enable/disable CanAirIO API state
    if (act.equals("ast")) {
      preferences.begin(_app_name, false);
      preferences.putBool("apiEnable", aenb);
      preferences.end();
      apiEnable = aenb;
      Serial.println("-->[CONFIG] Updating API state: "+aenb);
    }
  }
  else {
    Serial.println("-->[E][CONFIG] invalid config file!");
    return false;
  }
  return true;
}

bool ConfigApp::isWifiEnable(){
  return wifiEnable;
}

bool ConfigApp::isApiEnable(){
  return apiEnable;
}

bool ConfigApp::isIfxEnable(){
  return ifxEnable;
}

void ConfigApp::reboot(){
  delay(100);
  ESP.restart();
}