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

void GUIUtils::showProgress(unsigned int progress, unsigned int total){
  u8g2.setFont(u8g2_font_4x6_tf);
  char output[12];
  sprintf(output, "%03d%%" , (progress / (total / 100)));
  u8g2.drawStr(0, lastDrawedLine, output);
  u8g2.sendBuffer();
}

void GUIUtils::welcomeAddMessage(String msg){
  u8g2.setFont(u8g2_font_4x6_tf);

#ifdef TTGO_TQ
   if (lastDrawedLine<32) {
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

void GUIUtils::displayCenterBig(String msg){
  
#ifdef TTGO_TQ
  u8g2.setCursor(0,1);
  u8g2.setFont(u8g2_font_inb30_mn);
#else
  u8g2.setCursor(0,0);
  u8g2.setFont(u8g2_font_inb24_mn);
#endif

  u8g2.print(msg.c_str());
}

void GUIUtils::displayBottomLine(String msg){
  u8g2.setFont(u8g2_font_4x6_tf);
  
#ifdef TTGO_TQ
  u8g2.setCursor(97,5);
#else
  u8g2.setCursor(0,29);
#endif

  u8g2.print(msg.c_str());
}

void GUIUtils::displayEndLine(String msg){
  u8g2.setFont(u8g2_font_5x7_tf);

#ifdef TTGO_TQ
  u8g2.setCursor(81, 14);
#else
  u8g2.setCursor(0, 41);
#endif

  u8g2.print(msg.c_str());
}

void GUIUtils::displaySensorAvarage(int avarage){
  char output[4];
  sprintf(output, "%03d", avarage);
  displayCenterBig(output);
}

// TODO: separate this function, format/display
void GUIUtils::displaySensorData(int pm25, int pm10, int chargeLevel){
  if(mcount<65535)mcount++;
  else mcount=0;
  char output[22];
  sprintf(output, "%03d E%02d [S%05d]" , pm10, ecode, mcount);
  displayBottomLine(String(output));

#ifdef TTGO_TQ
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.setCursor(105,15);
  u8g2.print(mcount);
  u8g2.setCursor(80,5); 
  u8g2.print(chargeLevel);
#endif
  Serial.print(" PM10:"); Serial.print(output);
  sprintf(output, "P%03d" , pm25);
  displayEndLine(String(output));
  Serial.print(" PM2.5:"); Serial.println(output);
}

void GUIUtils::displayStatus(bool wifiOn, bool bleOn, bool blePair, bool dataOn){


#ifdef TTGO_TQ

  if(bleOn) u8g2.drawBitmap(115, 24, 1, 8, ic_bluetooth_on);

  if(blePair) u8g2.drawBitmap(115, 24, 1, 8, ic_bluetooth_pair);
  
  if(wifiOn) u8g2.drawBitmap(101, 24, 1, 8, ic_wifi_on);

  if(dataOn) u8g2.drawBitmap(86, 24, 1, 8, ic_data_on);

#else 

  if(bleOn) u8g2.drawBitmap(54, 40, 1, 8, ic_bluetooth_on);

  if(blePair) u8g2.drawBitmap(54, 40, 1, 8, ic_bluetooth_pair);
  
  if(wifiOn) u8g2.drawBitmap(44, 40, 1, 8, ic_wifi_on);

  if(dataOn) u8g2.drawBitmap(34, 40, 1, 8, ic_data_on);

  u8g2.drawLine(0, 38, 63, 38);
  
#endif

}

void GUIUtils::updateError(unsigned int error){
  ecode = error;
}

void GUIUtils::pageStart(){
  u8g2.firstPage();
}

void GUIUtils::pageEnd(){
  u8g2.nextPage();
}