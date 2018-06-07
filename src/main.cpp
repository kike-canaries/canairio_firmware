
/**
 * @file example.ino
 * @author Felix Galindo
 * @date June 2017
 * @brief Example using HPMA115S0 sensor library on a Feather 32u4
 * @license MIT
 */

#include "Arduino.h"
#include <hpma115S0.h>
#include "SSD1306Wire.h"

//Create an instance of hardware serial
HardwareSerial hpmaSerial(1);
// Display via i2c for WeMOS OLED
SSD1306Wire display(0x3c, 5, 4);
// Create an instance of the hpma115S0 library
HPMA115S0 hpma115S0(hpmaSerial);

void showWelcome(){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, display.getHeight()/2, "ESP32 HPMA115S0");
  display.display();
  Serial.println("-->Welcome screen ready");
  delay(1000);
  display.setLogBuffer(5, 30);
}

void displayOnBuffer(String msg){
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.println(msg);
  display.drawLogBuffer(0,0);
  display.display();
}

void setup() {
  Serial.begin(9600);
  hpmaSerial.begin(9600,SERIAL_8N1,13,15);
  Serial.println("-->HardwareSerial ready");
  Serial.println("-->DebugConsole ready");
  delay(100);
  display.init();
  display.flipScreenVertically();
  display.setContrast(128);
  display.clear();
  Serial.println("-->OLED ready");
  Serial.println("Starting...");
  hpma115S0.Init();
  hpma115S0.StartParticleMeasurement();
  showWelcome();
}

void loop() {
  unsigned int pm2_5, pm10;
  if (hpma115S0.ReadParticleMeasurement(&pm2_5, &pm10)) {
    Serial.print("PM 2.5:\t" + String(pm2_5) + " ug/m3\t" );
    Serial.println("\tPM 10:\t" + String(pm10) + " ug/m3" );
    displayOnBuffer("PM25:  " + String(pm2_5) + " | PM10:  " + String(pm10));
  }
  delay(1000);
}
