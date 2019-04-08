/***
 * TESTING U8G2Lib
 * @hpsaturn 
 ***/

#include <Arduino.h>
#include <GUIUtils.hpp>
#include <ConfigApp.hpp>

#define D1MINI 1

#ifdef WEMOSOLED // display via i2c for WeMOS OLED board
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
#elif HELTEC // display via i2c for Heltec board
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 15, 4, 16);
#else        // display via i2c for D1MINI board
    U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);
#endif


GUIUtils gui;
ConfigApp cfg;

void setup(void) {
  Serial.begin(115200);
  Serial.println("\n== INIT SETUP ==\n");
  Serial.println("-->[SETUP] console ready");
  gui.displayInit(u8g2);
  gui.showWelcome();
  cfg.init("canairio");
  gui.welcomeAddMessage("ifid:"+String(cfg.ifxid));
  gui.welcomeAddMessage("ifpt:"+String(cfg.ifxpt));
  gui.welcomeAddMessage("espid:"+String((uint16_t)(cfg.chipid >> 32)));
  gui.welcomeAddMessage("wifie:"+String(cfg.wifiEnable));
  gui.welcomeAddMessage("==SETUP READY==");
  delay(1000);
}

void loop(void) {
  
}

