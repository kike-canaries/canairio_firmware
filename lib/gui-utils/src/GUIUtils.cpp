#include "GUIUtils.hpp"
#include "GUIIcons.h"

/******************************************************************************
*   D I S P L A Y  M E T H O D S
******************************************************************************/

void GUIUtils::displayInit(U8G2 &u8g2)
{
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

void GUIUtils::showWelcome()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(0, 0, "CanAirIO");
  u8g2.sendBuffer();
  String version = String(VERSION_CODE + VCODE);
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(46, 1, version.c_str());
  u8g2.drawLine(0, 9, 63, 9);
  u8g2.sendBuffer();
  lastDrawedLine = 12;
  // only for first screen
  Serial.println("-->[OLED] welcome screen ready.");
  u8g2.sendBuffer();
}

void GUIUtils::showProgress(unsigned int progress, unsigned int total)
{
  u8g2.setFont(u8g2_font_4x6_tf);
  char output[12];
  sprintf(output, "%03d%%", (progress / (total / 100)));
  u8g2.drawStr(0, lastDrawedLine, output);
  u8g2.sendBuffer();
}

void GUIUtils::welcomeAddMessage(String msg)
{
  u8g2.setFont(u8g2_font_4x6_tf);
#ifdef TTGO_TQ
  if (lastDrawedLine < 32)
  {
    u8g2.drawStr(0, lastDrawedLine, msg.c_str());
    lastDrawedLine = lastDrawedLine + 7;
    u8g2.sendBuffer();
  }
  else
  {
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

void GUIUtils::displayCenterBig(String msg)
{
#ifndef EMOTICONS
#ifdef TTGO_TQ
  u8g2.setCursor(0, 1);
  u8g2.setFont(u8g2_font_inb30_mn);
#else
  u8g2.setCursor(0, 0);
  u8g2.setFont(u8g2_font_inb24_mn);
#endif
  u8g2.print(msg.c_str());
#else
#ifdef TTGO_TQ
  u8g2.setCursor(52, 01); //(36,8)
  u8g2.setFont(u8g2_font_9x18B_tf);
  u8g2.print(msg.c_str());
#else
  u8g2.setCursor(46, 01); //(36,8)
  u8g2.setFont(u8g2_font_9x18B_tf);
  u8g2.print(msg.c_str());
#endif
#endif
}

void GUIUtils::displayBottomLine(String msg)
{
  u8g2.setFont(u8g2_font_4x6_tf);
#ifdef TTGO_TQ
#ifndef EMOTICONS
  u8g2.setCursor(115, 16);
#else
  u8g2.setCursor(115, 16); //Falta nueva visualizacion para Vis1 TTGO!!!!!!!!!
#endif
#else
  u8g2.setCursor(0, 29);
#endif
  u8g2.print(msg.c_str());
}

void GUIUtils::displayEndLine(String msg)
{
  u8g2.setFont(u8g2_font_5x7_tf);
#ifdef TTGO_TQ
  u8g2.setCursor(81, 14);
#else
  u8g2.setCursor(0, 41);
#endif
  u8g2.print(msg.c_str());
}

void GUIUtils::displayEmoticonLabel(int numsmile, String msg)
{
    u8g2.setFont(u8g2_font_unifont_t_emoticons);
    u8g2.drawGlyph(76, 12, numsmile);
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.setCursor(77, 17);
    u8g2.print(msg);
}

void GUIUtils::displaySensorAvarage(int avarage)
{
#ifndef EMOTICONS
#ifdef TTGO_TQ
  if (avarage < 13)
  {
    displayEmoticonLabel(0x0024,"GOOD");
  }
  else if (avarage < 36)
  {
    displayEmoticonLabel(0x0062,"MODERATE");
  }
  else if (avarage < 56)
  {
    displayEmoticonLabel(0x0032,"UNH SEN G");
  }
  else if (avarage < 151)
  {
    displayEmoticonLabel(0x0051,"UNHEALTY");
  }
  else if (avarage < 251)
  {
    displayEmoticonLabel(0x0053,"VERY UNH");
  }
  else
  {
    displayEmoticonLabel(0x0057,"HAZARDOUS");
  }
#else
  delay(1);
#endif
#else
  if (avarage < 13)
  {
#ifdef TTGO_TQ
    u8g2.drawXBM(1, 0, 32, 32, SmilefaceGood);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setCursor(40, 15);
    u8g2.print("GOOD/");
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.setCursor(70, 16);
    u8g2.print("green");
#else
    u8g2.drawXBM(0, 1, 32, 32, SmilefaceGood);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setCursor(40, 15);
    u8g2.print("GOOD");
#endif
  }
  else if (avarage < 36)
  {
#ifdef TTGO_TQ
    u8g2.drawXBM(1, 0, 32, 32, SmilefaceModerate);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setCursor(38, 15);
    u8g2.print("MODERATE/");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(92, 16);
    u8g2.print("yell");
#else
    u8g2.drawXBM(0, 1, 32, 32, SmilefaceModerate);
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(25, 29);
    u8g2.print("MODERATE");
#endif
  }
  else if (avarage < 56)
  {
#ifdef TTGO_TQ
    u8g2.drawXBM(1, 0, 32, 32, SmilefaceUnhealthySensitiveGroups);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setCursor(38, 15);
    u8g2.print("UNH SEN/");
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.setCursor(86, 16);
    u8g2.print("orang");
#else
    u8g2.drawXBM(0, 1, 32, 32, SmilefaceUnhealthySensitiveGroups);
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(30, 29);
    u8g2.print("UNH SEN");
#endif
  }
  else if (avarage < 151)
  {
#ifdef TTGO_TQ
    u8g2.drawXBM(1, 0, 32, 32, SmilefaceUnhealthy);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setCursor(40, 15);
    u8g2.print("UNHEAL/");
    u8g2.drawGlyph(52, 15, 0x00d1);
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.setCursor(82, 16);
    u8g2.print("red");
#else
    u8g2.drawXBM(0, 1, 32, 32, SmilefaceUnhealthy);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(29, 28);
    u8g2.print("UNHEAL");
    u8g2.drawGlyph(41, 28, 0x00d1);
#endif
  }
  else if (avarage < 251)
  {
#ifdef TTGO_TQ
    u8g2.drawXBM(1, 0, 32, 32, SmilefaceVeryUnhealthy);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setCursor(38, 15);
    u8g2.print("V UNHEA/");
    u8g2.drawGlyph(76, 15, 0x00d1);
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.setCursor(88, 16);
    u8g2.print("viol");
#else
    u8g2.drawXBM(0, 1, 32, 32, SmilefaceVeryUnhealthy);
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.setCursor(25, 30);
    u8g2.print("V UNHEA");
#endif
  }
  else
  {
#ifdef TTGO_TQ
    u8g2.drawXBM(1, 0, 32, 32, SmilefaceHazardous);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setCursor(40, 15);
    u8g2.print("HAZARD/");
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.setCursor(90, 16);
    u8g2.print("brown");
#else
    u8g2.drawXBM(0, 1, 32, 32, SmilefaceHazardous);
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(30, 29);
    u8g2.print("HAZARD");
#endif
  }
#endif
  char output[4];
  sprintf(output, "%03d", avarage);
  displayCenterBig(output);
}

// TODO: separate this function, format/display
void GUIUtils::displaySensorData(int pm25, int pm10, int chargeLevel, float humi, float temp)
{
  if (mcount < 65535)
    mcount++;
  else
    mcount = 0;
  char output[22];
  inthumi = (int)humi;
  inttemp = (int)temp;
  sprintf(output, "%03d E%02d H%02d%% T%02d%°C", pm25, ecode, inthumi, inttemp); // 000 E00 H00% T00°C
//#ifndef EMOTICONS
  displayBottomLine(String(output));
///#endif
#ifdef TTGO_TQ
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawFrame(100, 0, 27, 13);
  u8g2.drawBox(97, 4, 3, 5);
  u8g2.setDrawColor(0);
  u8g2.drawBox(102, 2, 24, 9);
  u8g2.setDrawColor(1);

  if (chargeLevel < 80)
  {
    u8g2.setCursor(80, 12);
  }
  if (chargeLevel > 24)
  {
    u8g2.drawBox(120, 2, 5, 9);
  }
  if (chargeLevel > 49)
  {
    u8g2.drawBox(114, 2, 5, 9);
  }
  if (chargeLevel > 74)
  {
    u8g2.drawBox(108, 2, 5, 9);
  }
  if (chargeLevel > 99)
  {
    u8g2.drawBox(102, 2, 5, 9);
    u8g2.setCursor(76, 12);
  }
  Serial.print(" PM2.5:");
  Serial.print(output);
  Serial.print(" Battery:");
  Serial.print(chargeLevel);
  Serial.println("%");
#else
  Serial.print(" PM2.5:");
  Serial.println(output);
#ifdef EMOTICONS
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.setCursor(51, 0);
  sprintf(output, "%03d", pm25);
  u8g2.print(output);
#endif
#endif
  //displayEndLine(String(output));
}

void GUIUtils::displayStatus(bool wifiOn, bool bleOn, bool blePair, bool dataOn)
{
#ifdef TTGO_TQ
  if (bleOn)
    u8g2.drawBitmap(119, 24, 1, 8, ic_bluetooth_on);
  if (blePair)
    u8g2.drawBitmap(119, 24, 1, 8, ic_bluetooth_pair);
  if (wifiOn)
    u8g2.drawBitmap(106, 24, 1, 8, ic_wifi_on);
  if (dataOn)
    u8g2.drawBitmap(93, 24, 1, 8, ic_data_on);
#else
  if (bleOn)
    u8g2.drawBitmap(54, 40, 1, 8, ic_bluetooth_on);
  if (blePair)
    u8g2.drawBitmap(54, 40, 1, 8, ic_bluetooth_pair);
  if (wifiOn)
    u8g2.drawBitmap(44, 40, 1, 8, ic_wifi_on);
  if (dataOn)
    u8g2.drawBitmap(34, 40, 1, 8, ic_data_on);
  u8g2.drawLine(0, 38, 63, 38);
#endif
}

void GUIUtils::displayLiveIcon()
{
#ifdef TTGO_TQ
  if (toggleLive)
    u8g2.drawBitmap(80, 25, 1, 8, ic_sensor_live);
#else
  if (toggleLive)
    u8g2.drawBitmap(0, 40, 1, 8, ic_sensor_live);
#endif
  toggleLive = !toggleLive;
}

void GUIUtils::displayPrefSaveIcon(bool enable)
{
#ifdef TTGO_TQ
  if (enable)
    u8g2.drawBitmap(71, 24, 1, 8, ic_pref_save);
#else
  if (enable)
    u8g2.drawBitmap(10, 40, 1, 8, ic_pref_save);
#endif
}

void GUIUtils::updateError(unsigned int error)
{
  ecode = error;
}

void GUIUtils::pageStart()
{
  u8g2.firstPage();
}

void GUIUtils::pageEnd()
{
  u8g2.nextPage();
}