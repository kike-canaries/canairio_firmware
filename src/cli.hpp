

bool setup_mode = false;
int setup_time = 30000;

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
  if (cfg.ssid.length() == 0)
    Serial.println("WiFi is not configured\r");
  else if (WiFi.status() != WL_CONNECTED)
    Serial.println("WiFi is not connected\r");
  else
    Serial.println("WiFi is configured and connected");
  Serial.printf("UART sensor type is: %d\r\n", cfg.getSensorType());
  Serial.printf("Sample time: %d\r\n", cfg.stime);
  Serial.println();
}

void wcli_reboot(String opts) {
  wd.execute();
}

/**
 * @brief WiFi CLI init and custom commands
 **/
void wifiCLIInit() {
  wcli.setCallback(new mESP32WifiCLICallbacks());
  wcli.begin();
  // Main Commands:
  wcli.term->add("reboot", &wcli_reboot, "\tperform a ESP32 reboot");
  wcli.term->add("stime", &wcli_stime, "\tset the sample time (seconds)");
  wcli.term->add("stype", &wcli_stype, "\tset the sensor type (UART)");
  wcli.term->add("exit", &wcli_exit, "\texit of the setup mode (Auto exit in 30 seg)");
  wcli.term->add("setup", &wcli_setup, "\ttype this to start to configure the device :D\n");
  // Configuration loop:
  // 30 seconds for reconfiguration or first use case.
  // for reconfiguration type disconnect and switch the "output" mode
  uint32_t start = millis();
  while (setup_mode || (millis() - start < setup_time)) wcli.loop();
  Serial.println();
  Serial.println(" SETUP MODE END");
}