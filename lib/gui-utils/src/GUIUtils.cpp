#include "GUIUtils.hpp"
#include "GUIIcons.h"

/******************************************************************************
*   D I S P L A Y  M E T H O D S
******************************************************************************/

void GUIUtils::displayInit(U8G2 &u8g2){
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setContrast(255);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  u8g2.setFontMode(0);
  this->u8g2 = u8g2;
  Serial.println("-->[OLED] display ready.");
}

void GUIUtils::showWelcome(){
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(0, 0, "CanAirIO");
  u8g2.sendBuffer();
  String version = String(VERSION_CODE+VCODE);
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(46, 1, version.c_str());
  u8g2.drawLine(0, 9, 63, 9);
  u8g2.sendBuffer();
  lastDrawedLine = 12;
  // only for first screen
  Serial.println("-->[OLED] welcome screen ready.");
  u8g2.sendBuffer();
}

void GUIUtils::showProgress(unsigned int progress, unsigned int total) {
  u8g2.setFont(u8g2_font_4x6_tf);
  char output[12];
  sprintf(output, "%03d%%" , (progress / (total / 100)));
  u8g2.drawStr(0, lastDrawedLine, output);
  u8g2.sendBuffer();
}

void GUIUtils::welcomeAddMessage(String msg) {
  u8g2.setFont(u8g2_font_4x6_tf);
#ifdef TTGO_TQ
  if (lastDrawedLine < 32) {
    u8g2.drawStr(0, lastDrawedLine, msg.c_str());
    lastDrawedLine = lastDrawedLine + 7;
    u8g2.sendBuffer();
  }
  else {
    u8g2.drawStr(72, lastDrawedLine - 20, msg.c_str());
    lastDrawedLine = lastDrawedLine + 7;
    u8g2.sendBuffer();
  }
#else
  u8g2.drawStr(0, lastDrawedLine, msg.c_str());
  lastDrawedLine = lastDrawedLine + 7;
  u8g2.sendBuffer();
#endif
}

void GUIUtils::displayCenterBig(String msg) {
#ifdef TTGO_TQ
  u8g2.setCursor(0,1);
  u8g2.setFont(u8g2_font_inb30_mn);
#else
  u8g2.setCursor(0,0);
  u8g2.setFont(u8g2_font_inb24_mn);
#endif
  u8g2.print(msg.c_str());
}

void GUIUtils::displayBottomLine(String msg) {
  u8g2.setFont(u8g2_font_4x6_tf);
#ifdef TTGO_TQ
  u8g2.setCursor(80,0);   ////
#else
  u8g2.setCursor(0,29);
#endif
  u8g2.print(msg.c_str());
}

void GUIUtils::displayEndLine(String msg) {
  u8g2.setFont(u8g2_font_5x7_tf);
#ifdef TTGO_TQ
  u8g2.setCursor(81, 14);
#else
  u8g2.setCursor(0, 41);
#endif
  u8g2.print(msg.c_str());
}

void GUIUtils::displaySensorAvarage(int avarage) {
  char output[4];
  sprintf(output, "%03d", avarage);
  displayCenterBig(output);
}

void GUIUtils::displaySensorData(int pm25, int pm10, float humi, float temp) {
  displaySensorData(pm25,pm10,0,humi,temp);
}

// TODO: separate this function, format/display
void GUIUtils::displaySensorData(int pm25, int pm10, int chargeLevel, float humi, float temp) {
  if(mcount<65535)mcount++;
  else mcount=0;
  char output[22];
  inthumi = int(humi);
  inttemp = int(temp);
  sprintf(output, "%03d E%02d H%02d%% T%02d%%°C" , pm25, ecode, inthumi, inttemp);    // 000 E00 H00% T00°C
  displayBottomLine(String(output));
#ifdef TTGO_TQ
  u8g2.setFont(u8g2_font_4x6_tf);
 // u8g2.drawStr(112, 15, "T");   //Optional temperature
 // u8g2.setCursor(116, 15);
 // u8g2.print(u8x8_u8toa(inttemp, 2));
 // u8g2.drawStr(124, 15, "C");
  u8g2.drawFrame(100,9,27,13);
  u8g2.drawBox(97,13,3,5);
  u8g2.setDrawColor(0);
  u8g2.drawBox(102,11,24,9);
  u8g2.setDrawColor(1);
  u8g2.drawStr(89, 12, "%");
  if(chargeLevel<80){
     u8g2.setCursor(80, 12);
     u8g2.print(chargeLevel);
  }
  if(chargeLevel>24){
     u8g2.drawBox(120,11,5,9);
  }
  if(chargeLevel>49){
     u8g2.drawBox(114,11,5,9);
  }
  if(chargeLevel>74){
     u8g2.drawBox(108,11,5,9);
  }
  if(chargeLevel>99){
     u8g2.drawBox(102,11,5,9);
     u8g2.setCursor(76, 12);
     u8g2.print(chargeLevel);  
  }
  Serial.print(" PM2.5:"); Serial.print(output); Serial.print (" Battery:");
  Serial.print(chargeLevel); Serial.println("%");
#else
  Serial.print(" PM2.5:"); Serial.println(output);
#endif  
  //displayEndLine(String(output));
}

void GUIUtils::displayStatus(bool wifiOn, bool bleOn, bool blePair, bool dataOn) {
#ifdef TTGO_TQ
  if(bleOn) u8g2.drawBitmap(119, 24, 1, 8, ic_bluetooth_on);
  if(blePair) u8g2.drawBitmap(119, 24, 1, 8, ic_bluetooth_pair);
  if(wifiOn) u8g2.drawBitmap(106, 24, 1, 8, ic_wifi_on);
  if(dataOn) u8g2.drawBitmap(93, 24, 1, 8, ic_data_on);
#else 
  if(bleOn) u8g2.drawBitmap(54, 40, 1, 8, ic_bluetooth_on);
  if(blePair) u8g2.drawBitmap(54, 40, 1, 8, ic_bluetooth_pair);
  if(wifiOn) u8g2.drawBitmap(44, 40, 1, 8, ic_wifi_on);
  if(dataOn) u8g2.drawBitmap(34, 40, 1, 8, ic_data_on);
  u8g2.drawLine(0, 38, 63, 38);
#endif
}

void GUIUtils::displayLiveIcon() {
#ifdef TTGO_TQ
  if(toggleLive)u8g2.drawBitmap(80, 24, 1, 8, ic_sensor_live);
#else
  if(toggleLive)u8g2.drawBitmap(0, 40, 1, 8, ic_sensor_live);
#endif  
  toggleLive=!toggleLive;
}

void GUIUtils::displayPrefSaveIcon(bool enable) {
#ifdef TTGO_TQ
  if(enable)u8g2.drawBitmap(71, 24, 1, 8, ic_pref_save);
#else
  if(enable)u8g2.drawBitmap(10, 40, 1, 8, ic_pref_save);
#endif  
}

void GUIUtils::updateError(unsigned int error) {
  ecode = error;
}

void GUIUtils::pageStart() {
  u8g2.firstPage();
}

void GUIUtils::pageEnd() {
  u8g2.nextPage();
}