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
  u8g2.firstPage();  // only for first screen
#ifdef D1MINI
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(0, 0, "CanAirIO");
  String version = "("+String(VERSION_CODE+VCODE)+")";
  u8g2.drawStr(0, 8,version.c_str());
  u8g2.drawLine(0, 16, 63, 16);
#else
  String version = "CanAirIO ("+String(VERSION_CODE+VCODE)+")";
  u8g2.drawStr(0, 0,version.c_str());
  u8g2.drawLine(0, 11, 128, 11);
#endif
  // only for first screen
  Serial.println("-->[OLED] welcome screen ready.");
  u8g2.nextPage();
}

void GUIUtils::displayCenterBig(String msg){
#ifdef D1MINI
  u8g2.setCursor(0,0);
  u8g2.setFont(u8g2_font_inb24_mn);
#else
  u8g2.setCursor(73,40);
  u8g2.setFont(u8g2_font_freedoomr25_mn);
#endif
  u8g2.print(msg.c_str());
}

void GUIUtils::displayBottomLine(String msg){
  u8g2.setFont(u8g2_font_4x6_tf);
#ifdef D1MINI
  u8g2.setCursor(0, 29);
  u8g2.print(msg.c_str());
#else
  u8g2.setCursor(0, 16);
  u8g2.print(msg.c_str());
#endif
}

void GUIUtils::displayEndLine(String msg){
  u8g2.setFont(u8g2_font_5x7_tf);
#ifdef D1MINI
  u8g2.setCursor(0, 41);
  u8g2.print(msg.c_str());
#else
  u8g2.setCursor(0, 16);
  u8g2.print(msg.c_str());
#endif
}

void GUIUtils::displaySensorAvarage(int avarage){
  char output[4];
  sprintf(output, "%03d", avarage);
  displayCenterBig(output);
}

void GUIUtils::displaySensorData(int pm25, int pm10){
  if(mcount<65535)mcount++;
  else mcount=0;
#ifdef D1MINI
  char output[22];
  sprintf(output, "%03d E%02d [S%05d]" , pm10, ecount, mcount);
#else
  sprintf(output, "%04d P25:%03d P10:%03d", mcount, pm25, pm10);
#endif
  displayBottomLine(String(output));
  Serial.print(" PM10:"); Serial.print(output);
  sprintf(output, "P%03d" , pm25);
  displayEndLine(String(output));
  Serial.print(" PM2.5:"); Serial.println(output);
}

void GUIUtils::displayStatus(bool wifiOn, bool bleOn, bool blePair, bool dataOn){

  if(bleOn) u8g2.drawBitmap(54, 40, 1, 8, ic_bluetooth_on);

  if(blePair) u8g2.drawBitmap(54, 40, 1, 8, ic_bluetooth_pair);
  
  if(wifiOn) u8g2.drawBitmap(44, 40, 1, 8, ic_wifi_on);

  if(dataOn) u8g2.drawBitmap(34, 40, 1, 8, ic_data_on);

  u8g2.drawLine(0, 38, 63, 38);

}

void GUIUtils::updateError(){
  ecount++;
  if(ecount>99)ecount=0;
}

void GUIUtils::pageStart(){
  u8g2.firstPage();
}

void GUIUtils::pageEnd(){
  do; while(u8g2.nextPage());
}