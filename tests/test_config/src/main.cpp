/***
 * TESTING ConfigApp library
 * @hpsaturn 
 ***/

#include <Arduino.h>
#include <GUIUtils.hpp>
#include <ConfigApp.hpp>

void setup(void) {
  Serial.begin(115200);
  Serial.println("\n== INIT SETUP ==\n");
  Serial.println("-->[SETUP] console ready");
  gui.displayInit();
  gui.showWelcome();
  cfg.init("canairio");
  gui.welcomeAddMessage("ifdb:"+String(cfg.ifxdb));
  gui.welcomeAddMessage("ifip:"+String(cfg.ifxip));
  gui.welcomeAddMessage("espid:"+String((uint16_t)(cfg.chipid >> 32)));
  gui.welcomeAddMessage("ssid:"+String(cfg.ssid));
  gui.welcomeAddMessage("==SETUP READY==");
  delay(1000);
}

void loop(void) {
  
}

