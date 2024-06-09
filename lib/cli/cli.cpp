#ifndef DISABLE_CLI
#include "cli.hpp"
#include <wifi.hpp>
#include "cloud_influxdb.hpp"
#include "ConfigApp.hpp"

bool setup_mode = false;
int setup_time = 10000;
bool first_run = true;

TaskHandle_t xCliHandle;

void wcli_debug(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String param = operands.first();
  param.toUpperCase();
  bool dbgmode = param.equals("ON") || param.equals("1");
  debugEnable(dbgmode);
  devmode = dbgmode;
  sensors.setDebugMode(dbgmode);
  battery.debug = dbgmode;
}

bool isValidKey(String key) {
  for (int i = 0; i < KCOUNT; i++) {
    if (key.equals(cfg.getKey((CONFKEYS)i))) return true;
  }
  return false;
}

String getValue(String key) {
  ConfKeyType type = cfg.getKeyType(key);
  if (type == ConfKeyType::BOOL) return cfg.getBool(key, false) ? "true" : "false";
  if (type == ConfKeyType::FLOAT) return String(cfg.getFloat(key, false));
  if (type == ConfKeyType::INT) return String(cfg.getInt(key, false));
  if (type == ConfKeyType::STRING) return cfg.getString(key, "");
  return "";
}

void wcli_klist(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String opt = operands.first();
  int key_count = KCOUNT;                       // Show all keys to configure 
  if (opt.equals("basic")) key_count = KBASIC; // Only show the basic keys to configure
  Serial.printf("\n%11s \t%s \t%s \r\n", "KEYNAME", "DEFINED", "VALUE");
  Serial.printf("\n%11s \t%s \t%s \r\n", "=======", "=======", "=====");

  for (int i = 0; i < key_count; i++) {
    if (i == KBASIC) continue;
    if (i == PKEYS::KPASS) continue;
    String key = cfg.getKey((CONFKEYS)i);
    bool isDefined = cfg.isKey(key);
    String defined = isDefined ? "custom " : "default";
    String value = "";
    if (isDefined) value = getValue(key);
    Serial.printf("%11s \t%s \t%s \r\n", key, defined.c_str(), value.c_str());
  }

  Serial.printf("\r\nMore info: https://canair.io/docs/cli\r\n");
}

void saveInteger(String key, String v) {
  int32_t value = v.toInt();
  cfg.saveInt(key, value);
  Serial.printf("saved: %s:%i\r\n",key.c_str(),value);
}

void saveFloat(String key, String v) {
  float value = v.toFloat();
  cfg.saveFloat(key, value);
  Serial.printf("saved: %s:%.5f\r\n",key.c_str(),value);
}

void saveBoolean(String key, String v) {
  v.toLowerCase();
  cfg.saveBool(key, v.equals("on") || v.equals("1") || v.equals("enable") || v.equals("true"));
  Serial.printf("saved: %s:%s\r\n", key.c_str(), cfg.getBool(key, false) ? "true" : "false");
}

void saveString(String key, String v) {
  cfg.saveString(key, v);
  Serial.printf("saved: %s:%s\r\n",key.c_str(),v.c_str());
}

void wcli_kset(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String key = operands.first();
  String v = operands.second();
  if(isValidKey(key)){
    if(cfg.getKeyType(key) == ConfKeyType::BOOL) saveBoolean(key,v);
    else if(cfg.getKeyType(key) == ConfKeyType::FLOAT) saveFloat(key,v);
    else if(cfg.getKeyType(key) == ConfKeyType::INT) saveInteger(key,v);
    else if(cfg.getKeyType(key) == ConfKeyType::STRING) saveString(key,v);
    else Serial.println("Invalid key action for: " + key);
  }
  else {
    Serial.printf("invalid key: %s\r\nPlease see the valid keys with klist command.\r\n",key.c_str());
  }
}

void wcli_uartpins(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  if (operands.first().isEmpty() || operands.second().isEmpty()) {
    Serial.printf("current TX/RX configured: %i/%i\r\n", sTX, sRX);
    return;
  }
  int sTX = operands.first().toInt();
  int sRX = operands.second().toInt();
  if (sTX >= 0 && sRX >= 0) {
    saveSensorPins(sTX, sRX);
  }
  else
    Serial.println("invalid pins values");
}

bool validBattLimits(float min, float max){
  return (min >= 3.0 && min <= 5.0 && max <=5.0 && max >= 3.0);
}

void wcli_battvLimits(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  float battMin = operands.first().toFloat();
  float battMax = operands.second().toFloat();
  if (validBattLimits(battMin, battMax)) {
    Serial.printf("Battery limits: Vmin: %2.2f Vmax: %2.2f\r\n", battMin, battMax);
    battery.setBattLimits(battMin, battMax);
    cfg.saveFloat(CONFKEYS::KBATVMI,battMin);
    cfg.saveFloat(CONFKEYS::KBATVMX,battMax);
  }
  else {
    Serial.println("-->[BATT] !invalid battery value! Current values:");
    battery.printLimits();
  }
}

void wcli_chargLimits(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  float battMin = operands.first().toFloat();
  float battMax = operands.second().toFloat();
  if (validBattLimits(battMin, battMax)) {
    Serial.printf("Battery charging limits: Vmin: %2.2f Vmax: %2.2f\r\n", battMin, battMax);
    battery.setChargLimits(battMin, battMax);
    cfg.saveFloat(CONFKEYS::KCHRVMI,battMin);
    cfg.saveFloat(CONFKEYS::KCHRVMX,battMax);
  }
  else {
    Serial.println("-->[BATT] !invalid battery value! Current values:");
    battery.printLimits();
  }
}

void wcli_stime(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  int stime = operands.first().toInt();
  if (stime >= 5) {
    saveSampleTime(stime);
    sensors.setSampleTime(stime);
  }
  else 
    Serial.println("invalid sample time");
}

void wcli_stype_error(){
  Serial.println("invalid UART sensor type! Choose one into 0-7:");
  for (int i=0; i<=7 ;i++)Serial.printf("%i\t%s\r\n",i,sensors.getSensorName((SENSORS)i));
}

void wcli_stype(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String stype = operands.first();
  if(stype.length()==0){
    wcli_stype_error();
    return;
  }
  int type = stype.toInt();
  if (type > 7 || type < 0) wcli_stype_error();
  else {
    saveSensorType(type);
    Serial.printf("\nselected UART sensor model\t: %s\r\n", sensors.getSensorName((SENSORS)type));
    Serial.println("Please reboot to changes apply");
  }
}

void wcli_sgeoh (char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String geoh = operands.first();
  if (geoh.length() > 5) {
    geoh.toLowerCase(); 
    saveGeo(geoh);
    ifxdbEnable(true);
  } else {
    Serial.println("\nInvalid Geohash. (Precision should be > to 5).\r\n");
    Serial.println("Please visit: http://bit.ly/geohashe");
    Serial.println("\nand select one of your fixed station.");
  }
}

void wcli_sensors() {
    Serial.printf("\r\nCanAirIO Sensorslib\t: %s\r\n",sensors.getLibraryVersion().c_str());
    int i = 0;
    int count = sensors.getSensorsRegisteredCount();
    Serial.printf("Sensors count  \t\t: %i (", count);
    if (count > 0 && sensors.getSensorsRegistered()[0] == SENSORS::Auto) {
      Serial.printf("%s,", sensors.getSensorName((SENSORS)sensors.getSensorsRegistered()[0]));
      i = 1;
    }
    while (sensors.getSensorsRegistered()[i++] != 0) {
      Serial.printf("%s,", sensors.getSensorName((SENSORS)sensors.getSensorsRegistered()[i-1]));
    }
    Serial.println(")");
}

void wcli_sensors_values() {
    Serial.println("\r\nCurrent sensors values:");
    UNIT unit = sensors.getNextUnit();
    while(unit != UNIT::NUNIT) {
        String uName = sensors.getUnitName(unit);
        float uValue = sensors.getUnitValue(unit);
        String uSymb = sensors.getUnitSymbol(unit);
        Serial.printf(" %s:\t%02.1f\t%s\r\n", uName.c_str(), uValue, uSymb.c_str());
        unit = sensors.getNextUnit();
    }
}

void wcli_info(char *args, Stream *response) {
  Serial.println();
  Serial.print(getDeviceInfo());
  wcli_sensors();
  wcli_sensors_values();
}

void wcli_exit(char *args, Stream *response) {
  setup_time = 0;
  setup_mode = false;
}

void wcli_setup(char *args, Stream *response) {
  setup_mode = true;
  Serial.println("\r\nSetup Mode. Main presets:\r\n");
  String canAirIOname = "Please set your geohash with \"sgeoh\" cmd";
  if(geo.length()>5)canAirIOname = getStationName();
  Serial.printf("CanAirIO device id\t: %s\r\n", canAirIOname.c_str());
  Serial.printf("Device factory id\t: %s\r\n", getAnaireDeviceId().c_str());
  Serial.printf("Sensor geohash id\t: %s\r\n", geo.length() == 0 ? "undefined" : geo.c_str());
  Serial.printf("WiFi current status\t: %s\r\n", WiFi.status() == WL_CONNECTED ? "connected" : "disconnected");
  Serial.printf("Sensor sample time \t: %d\r\n", stime);
  Serial.printf("UART sensor model \t: %s\r\n", sensors.getSensorName((SENSORS)stype));
  Serial.printf("UART sensor TX pin\t: %d\r\n", sTX == -1 ? PMS_TX : sTX);
  Serial.printf("UART sensor RX pin\t: %d\r\n", sRX == -1 ? PMS_RX : sRX);
  Serial.printf("Current debug mode\t: %s\r\n", devmode == true ? "enabled" : "disabled");

  wcli_klist((char *)"basic",response);

  Serial.printf("\r\nType \"klist\" for advanced settings\r\n");
  Serial.printf("Type \"help\" for available commands details\r\n");
  Serial.printf("Type \"exit\" for leave the safe mode\r\n");
}

void wcli_reboot(char *args, Stream *response) {
  wd.execute();
}

void wcli_clear(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String deviceId = operands.first();
  if (deviceId.equals(getAnaireDeviceId())) {
    Serial.println("Clearing device to defaults..");
    wcli.clearSettings();
    cfg.clear();
  }
  else {
    Serial.println("\nPlease type clear and the factory device id to confirm.");
  }
}

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
    
  }

  void onHelpShow() {
    // Enter your custom help here:
    Serial.println("\r\nCanAirIO Commands:\r\n");
    Serial.println("reboot\t\t\tperform a soft ESP32 reboot");
    Serial.println("clear\t\t\tfactory settings reset. (needs confirmation)");
    Serial.println("debug\t<on/off>\tto enable debug mode");
    Serial.println("stime\t<time>\t\tset the sample time in seconds");
    Serial.println("stype\t<sensor_type>\tset the UART sensor type. Integer.");
    Serial.println("sgeoh\t<GeohashId>\tset geohash id. Choose it here http://bit.ly/geohashe");
    Serial.println("spins\t<TX> <RX>\tset the UART pins");
    Serial.println("battv\t<Vmin> <Vmax>\tset battery min/max voltage");
    Serial.println("charg\t<Vmin> <Vmax>\tset battery charging min/max voltage");
    Serial.println("kset\t<key> <value>\tset preference key value (e.g on/off or 1/0 or text)");
    Serial.println("klist\t\t\tlist valid preference keys");
    Serial.println("info\t\t\tget the device information");
    Serial.println("exit\t\t\texit of the initial setup mode");
    Serial.println("setup\t\t\ttype this to start the configuration");

    if(first_run) Serial.println("\n\nEnter the word: \"setup\" to configure the device");
    first_run = false;
  }

  void onNewWifi(String ssid, String passw){
    saveWifi(ssid,passw);
  }
};

void cliTask(void *param) {
  for ( ; ; ) {
    wcli.loop();
    vTaskDelay(120 / portTICK_PERIOD_MS);
  }
  vTaskDelete( NULL );
}

void cliTaskInit() {
#ifndef DISABLE_CLI
  xTaskCreatePinnedToCore(
      cliTask,     // Function to implement the task
      "cliTask ",  // Name of the task
      4000,        // Stack size in words
      NULL,        // Task input parameter
      1,           // Priority of the task
      &xCliHandle,        // Task handle.
      1            // Core where the task should run
  );
#endif
}

int32_t cliTaskStackFree(){
    return uxTaskGetStackHighWaterMark(xCliHandle);
}

// const char logo[] =
// "┏┓    ┏┓•  ┳┏┓\r\n"
// "┃ ┏┓┏┓┣┫┓┏┓┃┃┃\r\n"
// "┗┛┗┻┛┗┛┗┗┛ ┻┗┛\r\n"
// "              \r\n"
// "\r\n"
// "\r\n"
// ""
// ;

const char logo[] =
" .d8888b.                           d8888 d8b         8888888  .d88888b.  \r\n"
"d88P  Y88b                         d88888 Y8P           888   d88P\" \"Y88b \r\n"
"888    888                        d88P888               888   888     888 \r\n"
"888         8888b.  88888b.      d88P 888 888 888d888   888   888     888 \r\n"
"888            \"88b 888 \"88b    d88P  888 888 888P\"     888   888     888 \r\n"
"888    888 .d888888 888  888   d88P   888 888 888       888   888     888 \r\n"
"Y88b  d88P 888  888 888  888  d8888888888 888 888       888   Y88b. .d88P \r\n"
" \"Y8888P\"  \"Y888888 888  888 d88P     888 888 888     8888888  \"Y88888P\"  \r\n"
"                                                                          \r\n"
"\r\n"
""
;

/**
 * @brief WiFi CLI init and CanAirIO custom commands
 **/
void cliInit() {

  wcli.shell->attachLogo(logo);

  wcli.setCallback(new mESP32WifiCLICallbacks());
  wcli.setSilentMode(true);
  wcli.disableConnectInBoot();
  // Main Commands:
  wcli.add("reboot", &wcli_reboot, "\tperform a ESP32 reboot");
  wcli.add("clear", &wcli_clear, "\tfactory settings reset. (needs confirmation)");
  wcli.add("debug", &wcli_debug, "\tenable debug mode");
  wcli.add("stime", &wcli_stime, "\tset the sample time (seconds)");
  wcli.add("stype", &wcli_stype, "\tset the sensor type (UART)");
  wcli.add("sgeoh", &wcli_sgeoh, "\tset geohash. Type help for more details.");
  wcli.add("spins", &wcli_uartpins, "\tset the UART pins TX RX");
  wcli.add("battv", &wcli_battvLimits, "\tset battery min/max voltage");
  wcli.add("charg", &wcli_chargLimits, "\tset battery charging min/max voltage");
  wcli.add("kset", &wcli_kset, "\tset preference key (e.g on/off or 1/0 or text)");
  wcli.add("klist", &wcli_klist, "\tlist valid preference keys");
  wcli.add("info", &wcli_info, "\tget device information");
  wcli.add("exit", &wcli_exit, "\texit of the setup mode. AUTO EXIT in 10 seg! :)");
  wcli.add("setup", &wcli_setup, "\tTYPE THIS WORD to enter to SAFE MODE setup");
  
  wcli.begin();

  // Configuration loop:
  // 10 seconds for reconfiguration or first use case.
  // for reconfiguration type disconnect and switch the "output" mode
  uint32_t start = millis();
  if (cfg.getBool(CONFKEYS::KFAILSAFE, true))
  {
    while (setup_mode || (millis() - start < setup_time)) wcli.loop();
    Serial.println();
    if (setup_time == 0)
      Serial.println("==>[INFO] Settings saved. Booting..\r\n");
    else
      Serial.println("==>[INFO] Time for initial setup over. Booting..\r\n");
  }
  Serial.println();
}
#endif