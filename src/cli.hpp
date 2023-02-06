#include <ConfigApp.hpp>

bool setup_mode = false;
int setup_time = 15000;

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
    
  }

  void onHelpShow() {
    // Enter your custom help here:
    Serial.println("\r\nCustom commands:\r\n");
    Serial.println("reboot\t\t\tperform a soft ESP32 reboot");
    Serial.println("\n\nenter the word: setup, to configure the device");
  }

  void onNewWifi(String ssid, String passw){
    cfg.saveWifi(ssid,passw);
    cfg.reload();
  }
};

void wcli_debug(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  String param = operands.first();
  param.toUpperCase();
  cfg.debugEnable(param.equals("ON") || param.equals("1"));
}

void wcli_uartpins(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int sTX = operands.first().toInt();
  int sRX = operands.second().toInt();
  if (sTX >= 0 && sRX >= 0) {
    cfg.saveSensorPins(sTX, sRX);
    cfg.reload();
  }
  else
    Serial.println("invalid pins values");
}
void wcli_stime(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int stime = operands.first().toInt();
  if (stime < 1) Serial.println("invalid sample time");
  else cfg.saveSampleTime(stime);
  cfg.reload();
}

void wcli_stype(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int stype = operands.first().toInt();
  if (stype > 7 || stype < 0) Serial.println("invalid UART sensor type");
  else cfg.saveSensorType(stype);
  cfg.reload();
}

void wcli_exit(String opts) {
  setup_time = 0;
  setup_mode = false;
}

void wcli_setup(String opts) {
  setup_mode = true;
  Serial.println("\r\nSetup status:\r\n");
  
  Serial.printf("WiFi current status\t: %s\r\n", WiFi.status() == WL_CONNECTED ? "connected" : "disconnected");
  Serial.printf("Sensor sample time \t: %d\r\n", cfg.stime);
  Serial.printf("UART sensor type \t: %d\r\n", cfg.getSensorType());
  Serial.printf("UART sensor RX   \t: %d\r\n", cfg.sRX);
  Serial.printf("UART sensor TX   \t: %d\r\n", cfg.sTX);
  Serial.printf("Sensor sample time\t: %d\r\n", cfg.stime);
  Serial.printf("Current debug mode\t: %s\r\n", cfg.devmode == true ? "enabled" : "disabled");
}

void wcli_reboot(String opts) {
  wd.execute();
}

/**
 * @brief WiFi CLI init and custom commands
 **/
void wifiCLIInit() {
  wcli.setCallback(new mESP32WifiCLICallbacks());
  wcli.setSilentMode(true);
  wcli.begin();
  // Main Commands:
  wcli.term->add("reboot", &wcli_reboot, "\tperform a ESP32 reboot");
  wcli.term->add("debug", &wcli_debug, "\tenable debug mode");
  wcli.term->add("stime", &wcli_stime, "\tset the sample time (seconds)");
  wcli.term->add("spins", &wcli_uartpins, "\tset the UART pins TX RX");
  wcli.term->add("stype", &wcli_stype, "\tset the sensor type (UART)");
  wcli.term->add("exit", &wcli_exit, "\texit of the setup mode (Auto exit in 15 seg)");
  wcli.term->add("setup", &wcli_setup, "\ttype this to start to configure the device :D\n");
  // Configuration loop:
  // 15 seconds for reconfiguration or first use case.
  // for reconfiguration type disconnect and switch the "output" mode
  uint32_t start = millis();
  while (setup_mode || (millis() - start < setup_time)) wcli.loop();
  Serial.println();
  Serial.println(" SETUP MODE END");
}