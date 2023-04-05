#include "GUIUtils.hpp"
#include "GUIIcons.h"

/******************************************************************************
*   D I S P L A Y  M E T H O D S
******************************************************************************/

void GUIUtils::displayInit() {
#ifdef WEMOSOLED  // display via i2c for WeMOS OLED board & TTGO18650
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
#elif HELTEC   // display via i2c for Heltec board
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 15, 4, 16);
#elif TTGO_TQ  // display via i2c for TTGO_TQ
    U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
#elif ESP32DEVKIT
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);
#elif ESP32PICOD4
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);
#elif M5PICOD4
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);
#else  // display via i2c for TTGO_T7 (old D1MINI) board
    U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);
#endif
    u8g2.setBusClock(100000);
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setContrast(128);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    u8g2.setFontMode(0);
    this->u8g2 = u8g2;
    dw = u8g2.getDisplayWidth();
    dh = u8g2.getDisplayHeight();

    // init battery (only for some boards)
    // batteryInit();

    Serial.println("-->[OGUI] display config ready.");
}

void GUIUtils::showWelcome() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(0, 0, "CanAirIO");
    u8g2.sendBuffer();
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(dw - 18, 1, getFirmwareVersionCode().c_str());
    u8g2.drawLine(0, 9, dw - 1, 9);
    lastDrawedLine = 10;
    // only for first screen
    u8g2.sendBuffer();
}

void GUIUtils::setPowerSave() {
    u8g2.setPowerSave(1);
}

void GUIUtils::setEmoticons(bool enable){
    emoticons = enable;
}

void GUIUtils::showMain() {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void GUIUtils::showProgress(unsigned int progress, unsigned int total) {
    u8g2.setFont(u8g2_font_4x6_tf);
    char output[12];
    sprintf(output, "%03d%%", (progress / (total / 100)));
    u8g2.drawStr(0, lastDrawedLine, output);
    u8g2.sendBuffer();
}

void GUIUtils::welcomeAddMessage(String msg) {
  if (lastDrawedLine >= dh - 6) {
    delay(500);
    showWelcome();
  }
  u8g2.setFont(u8g2_font_4x6_tf);
  if (dh == 32) {
    if (lastDrawedLine < 32) {
      u8g2.drawStr(0, lastDrawedLine, msg.c_str());
    } else {
      u8g2.drawStr(72, lastDrawedLine - 20, msg.c_str());
    }
  }
  else
    u8g2.drawStr(0, lastDrawedLine, msg.c_str());
  lastDrawedLine = lastDrawedLine + 7;
  u8g2.sendBuffer();
}

// TODO: This metod failed on redraw or clear the space first
void GUIUtils::welcomeRepeatMessage(String msg) {
    lastDrawedLine = lastDrawedLine - 7;
    welcomeAddMessage("               ");
    lastDrawedLine = lastDrawedLine - 7;
    welcomeAddMessage(msg);
}

void GUIUtils::displayUnit() {
  if (dw > 64) {
    u8g2.setFont(u8g2_font_6x13_tf);
    String unit = _unit_symbol;
    int strw = u8g2.getStrWidth(unit.c_str());
    u8g2.setCursor(dw-(strw+5), 40);
    u8g2.print(unit);
  }
}

void GUIUtils::displayCenterBig(String msg) {
  if (!emoticons) {
    #ifdef TTGO_TQ
    u8g2.setFont(u8g2_font_inb24_mn);
    #else
    if (dw > 64)
      u8g2.setFont(u8g2_font_inb33_mn);
    else
      u8g2.setFont(u8g2_font_inb19_mn);
    #endif
    int strw = u8g2.getStrWidth(msg.c_str());
    u8g2.setCursor((dw-strw)/2, 1);
    u8g2.print(msg.c_str());
  } else {
    #ifdef TTGO_TQ
    u8g2.setCursor(52, 00);
    u8g2.setFont(u8g2_font_9x18B_tf); 
    u8g2.print(msg.c_str());
    #else
    if (dw > 64) {
      if (_deviceType <= AQI_COLOR::AQI_PM) {  // PM
        u8g2.setCursor(dw - 64, 6);
        u8g2.setFont(u8g2_font_inb24_mn);
      } else {  // CO2
        u8g2.setCursor(dw - 62, 10);
        u8g2.setFont(u8g2_font_inb19_mn);
      }
    } else {
      if (_deviceType <= AQI_COLOR::AQI_PM) {  // PM
        u8g2.setCursor(dw - 28, 7);
        u8g2.setFont(u8g2_font_9x18B_tf);
      } else {  // CO2
        u8g2.setCursor(dw - 27, 8);
        u8g2.setFont(u8g2_font_7x13B_tf);
      }
    }
    u8g2.print(msg.c_str()); 
    #endif
  }
}

void GUIUtils::displayBottomLine(String msg) {
    #ifdef TTGO_TQ
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.setCursor(115, 16);
    u8g2.print(msg.c_str());
    #else
    if (!emoticons || dw > 64) {
      if (dw > 64) {
        u8g2.setFont(u8g2_font_7x13_tf);
        u8g2.setCursor(4, 40);
      }
      else {
        u8g2.setFont(u8g2_font_5x7_tf);
        int strw = u8g2.getStrWidth(msg.c_str());
        u8g2.setCursor((dw-strw)/2, 25);
      }
      u8g2.print(msg.c_str());
    }
    #endif
}

void GUIUtils::displayEmoticonLabel(int cursor, String msg) {
    u8g2.setFont(u8g2_font_unifont_t_emoticons);
    u8g2.drawGlyph(76, 12, cursor);
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.setCursor(77, 17);
    u8g2.print(msg);
}

void GUIUtils::displayBigEmoticon(String msg) {
  if (emoticons) {
    #ifdef TTGO_TQ
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setCursor(40, 14);
    u8g2.print(msg);
    #else
    u8g2.setFont(u8g2_font_5x7_tf);  //5x7 5x7 6x10 4x6 5x7
    u8g2.setCursor(29, 28);          //(35, 26);; (25, 29); (30, 29); (29, 28); (25, 30)(30, 29)
    u8g2.print(msg);                 //4 8 7 6 7 6
    #endif
  }
}

void GUIUtils::displayBigLabel(int cursor, String msg) {
  if (emoticons) {
    #ifdef TTGO_TQ
    u8g2.setFont(u8g2_font_5x7_tf);  //5x7 5x7 6x10 4x6 5x7
    u8g2.setCursor(cursor, 16);      //70 94 88 82 90 90
    u8g2.print(msg);
    #else
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.setCursor(35, 20);
    u8g2.print(msg);
    #endif  //4 8 7 6 7 6
  }
}

void GUIUtils::displayAQIColor(int average) {
if (!emoticons) {
#ifdef TTGO_TQ
    if (average < 13) {
      displayEmoticonLabel(0x0024, "GOOD");
    } else if (average < 36) {
      displayEmoticonLabel(0x0062, "MODERATE");
    } else if (average < 56) {
      displayEmoticonLabel(0x0032, "UNH SEN G");
    } else if (average < 151) {
      displayEmoticonLabel(0x0051, "UNHEALTY");
    } else if (average < 251) {
      displayEmoticonLabel(0x0053, "VERY UNH");
    } else {
      displayEmoticonLabel(0x0057, "HAZARDOUS");
    }
#endif 
  } 
  else {
    if (_deviceType <= AQI_COLOR::AQI_PM) {  //PM sensors and PAX
      if (average < 13) {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceGood);
        displayBigEmoticon("GOOD");
        displayBigLabel(66, "/green");
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceGood);
        displayBigEmoticon("  GOOD");
        displayBigLabel(0, " green");
#endif
      } else if (average < 36) {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceModerate);
        displayBigEmoticon("MODERATE");
        displayBigLabel(90, "/yel");
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceModerate);
        displayBigEmoticon("MODERATE");
        displayBigLabel(0, "yellow");
#endif
      } else if (average < 56) {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceUnhealthySGroups);
        displayBigEmoticon("UNH SEN");
        displayBigLabel(84, "/oran");
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceUnhealthySGroups);
        displayBigEmoticon("UNH SEN");
        displayBigLabel(0, "orange");
#endif
      } else if (average < 151) {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceUnhealthy);
        displayBigEmoticon("UNHEALT");
        displayBigLabel(84, "/red");  //OK
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceUnhealthy);
        displayBigEmoticon("UNHEALT");
        displayBigLabel(0, "  red");
#endif
      } else if (average < 251) {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceVeryUnhealthy);
        displayBigEmoticon("V UNHEA");
        displayBigLabel(84, "/viol");  //OK
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceVeryUnhealthy);
        displayBigEmoticon("V UNHEA");
        displayBigLabel(0, "violet");
#endif
      } else {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceHazardous);
        displayBigEmoticon("HAZARD");
        displayBigLabel(78, "/brown");
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceHazardous);
        displayBigEmoticon("HAZARD");
        displayBigLabel(0, " brown");
#endif
      } 
    } //PM sensors
    else {  
      if (average < 600) {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceGood);
        displayBigEmoticon("GOOD");
        displayBigLabel(66, "/green");
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceGood);
        displayBigEmoticon("  GOOD");
        displayBigLabel(0, " green");
#endif
      } 
      else if (average < 800) {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceModerate);
        displayBigEmoticon("MODERATE");
        displayBigLabel(90, "/yel");
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceModerate);
        displayBigEmoticon("MODERATE");
        displayBigLabel(0, "yellow");
#endif
      } 
      else if (average < 1000) {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceUnhealthy);
        displayBigEmoticon("UNHEALT");
        displayBigLabel(84, "/red");  //OK
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceUnhealthy);
        displayBigEmoticon("UNHEALT");
        displayBigLabel(0, "  red");
#endif
      } 
      else if (average < 1400) {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceVeryUnhealthy);
        displayBigEmoticon("V UNHEA");
        displayBigLabel(84, "/viol");  //OK
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceVeryUnhealthy);
        displayBigEmoticon("V UNHEA");
        displayBigLabel(0, "violet");
#endif
      } 
      else {
#ifdef TTGO_TQ
        u8g2.drawXBM(1, 0, 32, 32, SmileFaceHazardous);
        displayBigEmoticon("HAZARD");
        displayBigLabel(78, "/brown");
#else
        u8g2.drawXBM(0, 1, 32, 32, SmileFaceHazardous);
        displayBigEmoticon("HAZARD");
        displayBigLabel(0, " brown");
#endif
      }
    }
  }
}

void GUIUtils::displaySensorAverage(int average) {
  char output[6];
  if (_deviceType <= AQI_COLOR::AQI_PM) {  //PM sensors and PAX
      sprintf(output, "%03d", average);
  }
  else
      sprintf(output, "%04d", average);
  displayCenterBig(output);
}

void GUIUtils::displayWifiIcon(){
  int wifix = 10;
  if (dh == 32) wifix = 18;
  if (_rssi < 50)
    u8g2.drawBitmap(dw - wifix, dh - 8, 1, 8, ic_wifi_100);
  else if (_rssi < 60)
    u8g2.drawBitmap(dw - wifix, dh - 8, 1, 8, ic_wifi_75);
  else if (_rssi < 70)
    u8g2.drawBitmap(dw - wifix, dh - 8, 1, 8, ic_wifi_50);
  else
    u8g2.drawBitmap(dw - wifix, dh - 8, 1, 8, ic_wifi_25);
}

void GUIUtils::displayBatteryIcon(){
  if (_batteryCharge>80)
    u8g2.drawBitmap(0, dh - 8, 1, 8, ic_batt_100);
  else if (_batteryCharge>60)
    u8g2.drawBitmap(0, dh - 8, 1, 8, ic_batt_80);
  else if (_batteryCharge>40)
    u8g2.drawBitmap(0, dh - 8, 1, 8, ic_batt_60);
  else if (_batteryCharge>20)
    u8g2.drawBitmap(0, dh - 8, 1, 8, ic_batt_40);
  else
    u8g2.drawBitmap(0, dh - 8, 1, 8, ic_batt_00);
}

void GUIUtils::displayStatusBar(){
  // if (_batteryCharge == 0) {
  //   u8g2.print(" ");
  // } else {
  //   u8g2.setFont(u8g2_font_6x12_tf);
  //   u8g2.print(" ");
  //   _batteryCharge = abs(_batteryCharge);
  //   sprintf(output, "%02d", _batteryCharge);
  //   u8g2.print(_batteryCharge);
  //   u8g2.print("%");
  // }
}

void GUIUtils::displayMainValues() {
  displaySensorAverage(_average);
  displayAQIColor(_average);
  displayUnit();
  char output[20];
  sprintf(output, "%02d%% %2.1fC", (int)_humi,_temp);
  displayBottomLine(String(output)); 
  displayStatusBar();
  isNewData = false;
}

// TODO: separate this function, format/display
void GUIUtils::setSensorData(GUIData data) {
    suspendTaskGUI();
    _deviceType = data.color;
    _humi = data.humi;
    _temp = data.temp;
    _mainValue = data.mainValue;
    _minorValue = data.minorValue;
    _unit_symbol = data.unitSymbol;
    _unit_name = data.unitName;
    _average = _mainValue;
    _rssi = abs(data.rssi);
    isNewData = true;
    resumeTaskGUI();
}

void GUIUtils::setGUIStatusFlags(bool wifiOn, bool bleOn, bool blePair) {
    static uint_least64_t guiTimeStamp = 0;
    if (millis() - guiTimeStamp > 500) {
        guiTimeStamp = millis();
        suspendTaskGUI();
        _wifiOn = wifiOn;
        _bleOn = bleOn;
        _blePair = blePair;
        resumeTaskGUI();
    }
}

void GUIUtils::setInfoData(String info) {
    // TODO: 
}

void GUIUtils::setBatteryStatus(float volts, int charge, bool isCharging) {
     suspendTaskGUI();
    _batteryVolts = volts;
    _batteryCharge = abs(charge);
    _isCharging = isCharging;
    resumeTaskGUI();
}

void GUIUtils::displayGUIStatusFlags() {
#ifdef TTGO_TQ
    if (_blePair)
        u8g2.drawBitmap(dw - 9, dh - 8, 1, 8, ic_bluetooth_on);
    if (_wifiOn)
      displayWifiIcon();
    if (dataOn)
        u8g2.drawBitmap(dw - 35, dh - 8, 1, 8, ic_data_on);
    if (preferenceSave)
        u8g2.drawBitmap(dw - 57, dh - 8, 1, 8, ic_pref_save);
    if (sensorLive)
        u8g2.drawBitmap(dw - 48, dh - 8, 1, 8, ic_sensor_live);

#else
    u8g2.drawLine(0, dh - 11, dw - 1, dh - 11);
    if (_blePair)
        u8g2.drawBitmap(dw - 20, dh - 8, 1, 8, ic_bluetooth_on);
    if (_wifiOn)
        displayWifiIcon();
    if (dataOn)
        u8g2.drawBitmap(dw - 30, dh - 8, 1, 8, ic_data_on);
    if (preferenceSave)
        u8g2.drawBitmap(20, dh - 8, 1, 8, ic_pref_save);
    if (sensorLive)
        u8g2.drawBitmap(10, dh - 8, 1, 8, ic_sensor_live);
    displayBatteryIcon();
#endif
    if (dataOn) dataOn = false;                  // reset trigger for publish data ok.
    if (preferenceSave) preferenceSave = false;  // reset trigger for save preference ok.
    if (sensorLive) sensorLive = false;
}

/// enable trigger for show data ok icon, one time.
void GUIUtils::displayDataOnIcon() {
    dataOn = true;
}

/// enable trigger for sensor live icon, one time.
void GUIUtils::displaySensorLiveIcon() {
    sensorLive = true;
}

/// enable trigger for save preference ok, one time.
void GUIUtils::displayPreferenceSaveIcon() {
    preferenceSave = true;
}

void GUIUtils::pageStart() {
    u8g2.firstPage();
}

void GUIUtils::pageEnd() {
    u8g2.nextPage();
}

void GUIUtils::setBrightness(uint32_t value){

}

void GUIUtils::setWifiMode(bool enable){

}

void GUIUtils::setPaxMode(bool enable){

}

void GUIUtils::setSampleTime(int time){

}

void GUIUtils::setTrackValues(float speed, float distance){

}

void GUIUtils::setTrackTime(int h, int m, int s){

}

void GUIUtils::suspendTaskGUI(){
}

void GUIUtils::resumeTaskGUI(){
}

void GUIUtils::setCallbacks(GUIUserPreferencesCallbacks* pCallBacks){

}

void GUIUtils::loop(){
    static uint_least64_t guiTimeStamp = 0;
    if (millis() - guiTimeStamp > 500) {
        guiTimeStamp = millis();
        gui.pageStart();
        gui.displayMainValues();
        gui.displayGUIStatusFlags();
        gui.pageEnd();
    }
}

/// Firmware version from platformio.ini
String GUIUtils::getFirmwareVersionCode() {
    String VERSION_CODE = "r";
#ifdef REVISION
    int VCODE = REVISION;
#else
    int VCODE = 0;
#endif
    return String(VERSION_CODE + VCODE);
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_GUIHANDLER)
GUIUtils gui;
#endif
