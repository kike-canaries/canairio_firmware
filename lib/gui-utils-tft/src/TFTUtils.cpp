#include "TFTUtils.hpp"

/******************************************************************************
*   D I S P L A Y  M E T H O D S
******************************************************************************/


void TFTUtils::displayInit() {
    pinMode(0, INPUT_PULLUP);
    pinMode(35, INPUT);
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
    // ledcAttachPin(TFT_BL, pwmLedChannelTFT);
    // byte b = 1;
    // ledcWrite(pwmLedChannelTFT, backlight[b]);

    Serial.println("-->[OLED] display config ready.");
}

void TFTUtils::showWelcome() {
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setCursor(5, 5);
    tft.print("CanAirIO ");
    tft.println(getFirmwareVersionCode().c_str());
    lastDrawedLine = 20;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(1);
    Serial.println("-->[OLED] display welcome");
}

void TFTUtils::showProgress(unsigned int progress, unsigned int total) {
    Serial.println("-->[OLED] display progress");
    char output[12];
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.printf(output, "%03d%%", (progress / (total / 100)));
}

void TFTUtils::welcomeAddMessage(String msg) {
    Serial.println("-->[OLED] add message: "+msg);
    tft.setTextFont(1);
    tft.setCursor(5, lastDrawedLine);
    tft.println(msg.c_str());
    lastDrawedLine = lastDrawedLine + 10;
    delay(100);
}

//TODO: This metod failed on redraw or clear the space first
void TFTUtils::welcomeRepeatMessage(String msg) {
    lastDrawedLine = lastDrawedLine - 7;
    welcomeAddMessage("               ");
    lastDrawedLine = lastDrawedLine - 7;
    welcomeAddMessage(msg);
}

void TFTUtils::displayCenterBig(String msg, int deviceType) {
    tft.setFreeFont(&Orbitron_Light_32);
    tft.fillRect(3, 8, 120, 30, TFT_BLACK);
    tft.setCursor(5, 34);
    tft.print(msg.c_str());
    tft.setTextFont(0);
    if (deviceType <= 3)
        tft.println("ug/m3");
    else
        tft.println("ppm");
}

void TFTUtils::displayBottomLine(String msg) {
    
}

void TFTUtils::displayEmoticonLabel(int cursor, String msg) {
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setCursor(2, 187);
    tft.print(msg.c_str());
}

void TFTUtils::displayBigEmoticon(String msg) {

}

void TFTUtils::displayBigLabel(int cursor, String msg) {

}

void TFTUtils::displaySensorAverage(int average, int deviceType) {
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
    char output[4];
    sprintf(output, "%04d", average);
    displayCenterBig(output, deviceType);
}

// TODO: separate this function, format/display
void TFTUtils::displaySensorData(int mainValue, int chargeLevel, float humi, float temp, int rssi, int deviceType) {
    char output[22];
    if (deviceType <= 4)
        sprintf(output, "%04d E%02d H%02d%% T%02d°C", mainValue, 0, (int)humi, (int)temp);
    else
        sprintf(output, "%03d E%02d H%02d%% T%02d°C", mainValue, 0, (int)humi, (int)temp);
    displayBottomLine(String(output));
    
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setCursor(2, 227);
    // sprintf(output, "%04d", mainValue);
    tft.println(output);
}

void TFTUtils::displayStatus(bool wifiOn, bool bleOn, bool blePair) {
// #ifdef TTGO_TQ
//     if (bleOn)
//         u8g2.drawBitmap(dw - 9, dh - 8, 1, 8, ic_bluetooth_on);
//     if (blePair)
//         u8g2.drawBitmap(dw - 9, dh - 8, 1, 8, ic_bluetooth_pair);
//     if (wifiOn)
//         u8g2.drawBitmap(dw - 18, dh - 8, 1, 8, ic_wifi_on);
//     if (dataOn)
//         u8g2.drawBitmap(dw - 35, dh - 8, 1, 8, ic_data_on);
//     if (preferenceSave)
//         u8g2.drawBitmap(dw - 57, dh - 8, 1, 8, ic_pref_save);
//     if (sensorLive)
//         u8g2.drawBitmap(dw - 48, dh - 8, 1, 8, ic_sensor_live);

// #else
//     if (bleOn)
//         u8g2.drawBitmap(dw - 10, dh - 8, 1, 8, ic_bluetooth_on);
//     if (blePair)
//         u8g2.drawBitmap(dw - 10, dh - 8, 1, 8, ic_bluetooth_pair);
//     if (wifiOn)
//         u8g2.drawBitmap(dw - 20, dh - 8, 1, 8, ic_wifi_on);
//     if (dataOn)
//         u8g2.drawBitmap(dw - 30, dh - 8, 1, 8, ic_data_on);
//     if (preferenceSave)
//         u8g2.drawBitmap(10, dh - 8, 1, 8, ic_pref_save);
//     if (sensorLive)
//         u8g2.drawBitmap(0, dh - 8, 1, 8, ic_sensor_live);

//     u8g2.drawLine(0, dh - 10, dw - 1, dh - 10);
// #endif
//     if (dataOn) dataOn = false;                  // reset trigger for publish data ok.
//     if (preferenceSave) preferenceSave = false;  // reset trigger for save preference ok.
//     if (sensorLive) sensorLive = false;
}

/// enable trigger for show data ok icon, one time.
void TFTUtils::displayDataOnIcon() {
    dataOn = true;
}

/// enable trigger for sensor live icon, one time.
void TFTUtils::displaySensorLiveIcon() {
    sensorLive = true;
}

/// enable trigger for save preference ok, one time.
void TFTUtils::displayPreferenceSaveIcon() {
    preferenceSave = true;
}

void TFTUtils::pageStart() {
    tft.fillScreen(TFT_BLACK);
}

void TFTUtils::pageEnd() {
    // u8g2.nextPage();
}

/// Firmware version from platformio.ini
String TFTUtils::getFirmwareVersionCode() {
    String VERSION_CODE = "r";
#ifdef REVISION
    int VCODE = REVISION;
#else
    int VCODE = 0;
#endif
    return String(VERSION_CODE + VCODE);
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_GUIHANDLER)
TFTUtils gui;
#endif
