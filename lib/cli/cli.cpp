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
  #ifndef DISABLE_BATT
  battery.debug = dbgmode;
  #endif
}

void wcli_klist(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String opt = operands.first();
  int key_count = PKEYS::KCOUNT;                       // Show all keys to configure 
  if (opt.equals("basic")) key_count = PKEYS::KBASIC; // Only show the basic keys to configure
  response->printf("\n%11s \t%s \t%s \r\n", "KEYNAME", "DEFINED", "VALUE");
  response->printf("\n%11s \t%s \t%s \r\n", "=======", "=======", "=====");

  for (int i = 0; i < key_count; i++) {
    if (i == PKEYS::KBASIC) continue;
    if (i == PKEYS::KPASS) continue;
    String key = cfg.getKey((CONFKEYS)i);
    bool isDefined = cfg.isKey(key);
    String defined = isDefined ? "custom " : "default";
    String value = "";
    if (isDefined) value = cfg.getValue(key);
    response->printf("%11s \t%s \t%s \r\n", key, defined.c_str(), value.c_str());
  }

  response->printf("\r\nMore info: https://canair.io/docs/cli\r\n");
}

void wcli_kset(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String key = operands.first();
  String v = operands.second();
  if(cfg.saveAuto(key,v)){
    response->printf("saved key %s\t: %s\r\n", key, v);
  }
}

void wcli_uartpins(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  if (operands.first().isEmpty() || operands.second().isEmpty()) {
    response->printf("current TX/RX configured: %i/%i\r\n", sTX, sRX);
    return;
  }
  int sTX = operands.first().toInt();
  int sRX = operands.second().toInt();
  if (sTX >= 0 && sRX >= 0) {
    saveSensorPins(sTX, sRX);
  }
  else
    response->println("invalid pins values");
}

bool validBattLimits(float min, float max){
  return (min >= 3.0 && min <= 5.0 && max <=5.0 && max >= 3.0);
}

void wcli_battvLimits(char *args, Stream *response) {
  #ifndef DISABLE_BATT
  Pair<String, String> operands = wcli.parseCommand(args);
  float battMin = operands.first().toFloat();
  float battMax = operands.second().toFloat();
  if (validBattLimits(battMin, battMax)) {
    response->printf("Battery limits: Vmin: %2.2f Vmax: %2.2f\r\n", battMin, battMax);
    battery.setBattLimits(battMin, battMax);
    cfg.saveFloat(CONFKEYS::KBATVMI,battMin);
    cfg.saveFloat(CONFKEYS::KBATVMX,battMax);
  }
  else {
    response->println("-->[BATT] !invalid battery value! Current values:");
    battery.printLimits();
  }
  #endif
}

void wcli_chargLimits(char *args, Stream *response) {
  #ifndef DISABLE_BATT
  Pair<String, String> operands = wcli.parseCommand(args);
  float battMin = operands.first().toFloat();
  float battMax = operands.second().toFloat();
  if (validBattLimits(battMin, battMax)) {
    response->printf("Battery charging limits: Vmin: %2.2f Vmax: %2.2f\r\n", battMin, battMax);
    battery.setChargLimits(battMin, battMax);
    cfg.saveFloat(CONFKEYS::KCHRVMI,battMin);
    cfg.saveFloat(CONFKEYS::KCHRVMX,battMax);
  }
  else {
    response->println("-->[BATT] !invalid battery value! Current values:");
    battery.printLimits();
  }
  #endif
}

void wcli_stime(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  int stime = operands.first().toInt();
  if (stime >= 5) {
    saveSampleTime(stime);
    sensors.setSampleTime(stime);
  }
  else 
    response->println("invalid sample time");
}

void wcli_stype_error(Stream *response) {
  // SENSORS::SSCD30-1 is the seperator (see Sensors.hpp)
  response->printf("invalid UART sensor type! Choose one into 0-%i:\r\n",SENSORS::SSCD30-1);
  for (int i = 0; i <= SENSORS::SSCD30-1; i++) response->printf("%i\t%s\r\n", i, sensors.getSensorName((SENSORS)i));
}

void wcli_stype(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String stype = operands.first();
  if(stype.length()==0){
    wcli_stype_error(response);
    return;
  }
  int type = stype.toInt();
  if (type > SENSORS::SSCD30-1 || type < 0) wcli_stype_error(response); // SENSORS::SSCD30-1 is the seperator (see Sensors.hpp) 
  else {
    saveSensorType(type);
    response->printf("\nselected UART sensor model\t: %s\r\n", sensors.getSensorName((SENSORS)type));
    response->println("Please reboot to changes apply");
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
    response->println("\nInvalid Geohash. (Precision should be > to 5).\r\n");
    response->println("Please visit: http://bit.ly/geohashe");
    response->println("\nand select one of your fixed station.");
  }
}

void wcli_sensors(Stream *response) {
    response->printf("\r\nCanAirIO Sensorslib\t: %s\r\n",sensors.getLibraryVersion().c_str());
    int i = 0;
    int count = sensors.getSensorsRegisteredCount();
    response->printf("Sensors count  \t\t: %i (", count);
    if (count > 0 && sensors.getSensorsRegistered()[0] == SENSORS::Auto) {
      response->printf("%s,", sensors.getSensorName((SENSORS)sensors.getSensorsRegistered()[0]));
      i = 1;
    }
    while (sensors.getSensorsRegistered()[i++] != 0) {
      response->printf("%s,", sensors.getSensorName((SENSORS)sensors.getSensorsRegistered()[i-1]));
    }
    response->println(")");
}

void wcli_sensors_values(Stream *response) {
    response->println("\r\nCurrent sensors values:");
    UNIT unit = sensors.getNextUnit();
    while(unit != UNIT::NUNIT) {
        String uName = sensors.getUnitName(unit);
        float uValue = sensors.getUnitValue(unit);
        String uSymb = sensors.getUnitSymbol(unit);
        response->printf(" %s:\t%02.1f\t%s\r\n", uName.c_str(), uValue, uSymb.c_str());
        unit = sensors.getNextUnit();
    }
}

void wcli_info(char *args, Stream *response) {
  response->println();
  response->print(getDeviceInfo());
  wcli.status(response);
  wcli_sensors(response);
  wcli_sensors_values(response);
}

void wcli_exit(char *args, Stream *response) {
  setup_time = 0;
  setup_mode = false;
}

void wcli_setup(char *args, Stream *response) {
  setup_mode = true;
  response->println("\r\nSetup Mode. Main presets:\r\n");
  String canAirIOname = "Please set your geohash with \"sgeoh\" command";
  if(geo.length()>5)canAirIOname = getStationName();
  response->printf("CanAirIO device id\t: %s\r\n", canAirIOname.c_str());
  response->printf("Device factory id\t: %s\r\n", getAnaireDeviceId().c_str());
  response->printf("Sensor geohash id\t: %s\r\n", geo.length() == 0 ? "undefined" : geo.c_str());
  response->printf("WiFi current status\t: %s\r\n", WiFi.status() == WL_CONNECTED ? "connected" : "disconnected");
  response->printf("Sensor sample time \t: %d\r\n", stime);
  response->printf("UART sensor model \t: %s\r\n", sensors.getSensorName((SENSORS)stype));
  response->printf("UART sensor TX pin\t: %d\r\n", sTX == -1 ? PMS_TX : sTX);
  response->printf("UART sensor RX pin\t: %d\r\n", sRX == -1 ? PMS_RX : sRX);
  response->printf("Current debug mode\t: %s\r\n", devmode == true ? "enabled" : "disabled");

  wcli_klist((char *)"basic",response);

  response->printf("\r\nType \"klist\" for advanced settings\r\n");
  response->printf("Type \"help\" for available commands details\r\n");
  response->printf("Type \"info\" for sytem and sensors details\r\n");
}

void wcli_reboot(char *args, Stream *response) {
  wd.execute();
}

void wcli_swipe(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String deviceId = operands.first();
  if (deviceId.equals(getAnaireDeviceId())) {
    response->println("Clearing device to defaults..");
    wcli.clearSettings();
    cfg.clear();
  }
  else {
    response->println("\nPlease type clear and the factory device id. (showed in setup command)");
  }
}

void wcli_clear(char *args, Stream *response){
  wcli.shell->clear();
}

void cliTask(void *param) {
  for ( ; ; ) {
    wcli.loop();
    vTaskDelay(60 / portTICK_PERIOD_MS);
  }
  vTaskDelete( NULL );
}

void cliTaskInit() {
#ifndef DISABLE_CLI
  xTaskCreatePinnedToCore(cliTask, "cliTask ", 4000, NULL, 1, &xCliHandle, 1);
#endif
}

int32_t cliTaskStackFree(){
    return uxTaskGetStackHighWaterMark(xCliHandle);
}

const char logo[] =
"                                                                          \r\n"
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

void initRemoteShell(){
#ifndef DISABLE_CLI_TELNET 
  if (wcli.isTelnetEnable()) wcli.shellTelnet->attachLogo(logo);
#endif
}

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {}

  void onHelpShow() {}

  void onNewWifi(String ssid, String passw) { saveWifi(ssid, passw); }
};

void initShell(){
  wcli.shell->attachLogo(logo);
  wcli.setCallback(new mESP32WifiCLICallbacks());
  wcli.setSilentMode(true);
  wcli.disableAutoConnect();
  // Main Commands:
  wcli.add("reboot",&wcli_reboot,       "\tperform an ESP32 reboot");
  wcli.add("swipe", &wcli_swipe,        "\t\tfactory settings reset. (needs confirmation)");
  wcli.add("debug", &wcli_debug,        "\t\tenable debug mode");
  wcli.add("stime", &wcli_stime,        "\t\tset the sample time (seconds)");
  wcli.add("stype", &wcli_stype,        "\t\tset the sensor type (UART)");
  wcli.add("sgeoh", &wcli_sgeoh,        "\t\tset geohash. Type help for more details.");
  wcli.add("spins", &wcli_uartpins,     "\t\tset the UART pins TX RX");
  wcli.add("battv", &wcli_battvLimits,  "\t\tset battery min/max voltage");
  wcli.add("charg", &wcli_chargLimits,  "\t\tset battery charging min/max voltage");
  wcli.add("kset",  &wcli_kset,         "\t\tset preference key (e.g on/off or 1/0 or text)");
  wcli.add("klist", &wcli_klist,        "\t\tlist valid preference keys");
  wcli.add("info",  &wcli_info,         "\t\tget device information");
  wcli.add("exit",  &wcli_exit,         "\t\texit of the setup mode. AUTO EXIT in 10 seg! :)");
  wcli.add("clear", &wcli_clear,        "\t\tclear shell");
  wcli.add("setup", &wcli_setup,        "\t\tTYPE THIS WORD to enter to SAFE MODE setup");
  
  wcli.begin("CanAirIO");
}

/**
 * @brief WiFi CLI init and CanAirIO custom commands
 **/
void cliInit() {

  initShell();         // shell config and launched via Serial
  initRemoteShell();   // if it is enable, launched via Telnet
  
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