/******************************************************************************
*   C O N F I G  M E T H O D S
******************************************************************************/
String getConfigData(){
  StaticJsonDocument<300> doc;
  preferences.begin(app_name,false);
  doc["wenb"]   =  preferences.getBool("wifiEnable",false);
  doc["ssid"]   =  preferences.getString("ssid",""); // influxdb database name
  doc["ifxdb"]  =  preferences.getString("ifxdb",""); // influxdb database name
  doc["ifxip"]  =  preferences.getString("ifxip",""); // influxdb database ip
  doc["ifxid"]  =  preferences.getString("ifxid",""); // influxdb sensorid name
  doc["ifxtg"]  =  preferences.getString("ifxtg",""); // influxdb sensor tags
  doc["stime"]  =  preferences.getInt("stime",5);     // sensor measure time
  doc["wmac"]    =  (uint16_t)(chipid >> 32);
  preferences.end();
  String output;
  serializeJson(doc,output);
  return output;
}

void configInit(){
  preferences.begin(app_name,false);
  wifiEnable = preferences.getBool("wifiEnable",false);
  ssid = preferences.getString("ssid","");
  pass = preferences.getString("pass","");
  ifxdb = preferences.getString("ifxdb","");
  ifxip = preferences.getString("ifxip","");
  ifxid = preferences.getString("ifxid","");
  ifxtg = preferences.getString("ifxtg","");
  stime = preferences.getInt("stime",5);
  lat   = preferences.getDouble("lat",0);
  lon   = preferences.getDouble("lon",0);
  alt = preferences.getFloat("alt",0);
  spd = preferences.getFloat("spd",0);
  preferences.end();
}

void reboot() {
  Serial.println("-->[CONFIG] reboot..");
  delay(100);
  ESP.restart();
}

bool configSave(const char* json){
  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("-->[E][CONFIG] deserialize Json failed with code "));
    Serial.println(error.c_str());
    setErrorCode(ecode_json_parser_error);
    return false;
  }
  String tifxdb = doc["ifxdb"] | "";
  String tifxip = doc["ifxip"] | "";
  String tifxid = doc["ifxid"] | "";
  String tifxtg = doc["ifxtg"] | "";
  String tssid  = doc["ssid"]  | "";
  String tpass  = doc["pass"]  | "";
  int tstime    = doc["stime"] | 0;
  double tlat   = doc["lat"].as<double>();
  double tlon   = doc["lon"].as<double>();
  float talt    = doc["alt"].as<float>();
  float tspd    = doc["spd"].as<float>();
  uint16_t cmd  = doc["cmd"].as<uint16_t>();
  String act    = doc["act"]  | "";

  if (tifxdb.length()>0 && tifxip.length()>0 && tifxid.length()>0) {
    preferences.begin(app_name, false);
    preferences.putString("ifxdb", tifxdb );
    preferences.putString("ifxip", tifxip );
    preferences.putString("ifxid", tifxid );
    preferences.putString("ifxtg", tifxtg );
    preferences.end();
    Serial.println("-->[CONFIG] influxdb config saved!");
    Serial.print("-->[CONFIG] ");
    Serial.println(getConfigData());
  }
  else if (tssid.length()>0 && tpass.length()>0){
    preferences.begin(app_name, false);
    preferences.putString("ssid", tssid);
    preferences.putString("pass", tpass);
    preferences.putBool("wifiEnable",true);
    preferences.end();
    wifiEnable=true;
    isNewWifi=true;  // for execute wifi reconnect
    Serial.println("-->[AUTH] WiFi credentials saved!");
  }
  else if (tlat != 0 && tlon != 0) {
    preferences.begin(app_name, false);
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
    preferences.begin(app_name, false);
    preferences.putInt("stime", tstime);
    preferences.end();
    Serial.println("-->[CONFIG] sensor sample time saved!");
  }
  else if (cmd==((uint16_t)(chipid >> 32)) && act.length()>0){
    // reboot command
    if (act.equals("rbt")) {  
      reboot();
    }
    // clear preferences command
    if (act.equals("cls")) {
      preferences.begin(app_name, false);
      preferences.clear();
      preferences.end();
      reboot();
    }
    // wifi disable command
    if (act.equals("wst")) {
      preferences.begin(app_name, false);
      preferences.putBool("wifiEnable", false);
      preferences.end();
      wifiEnable = false;
      Serial.println("-->[CONFIG] disabling WiFi and radio..");
      wifiStop();
    }
  }
  else {
    Serial.println("-->[E][CONFIG] invalid config file!");
    setErrorCode(ecode_invalid_config);
    return false;
  }
  return true;
}