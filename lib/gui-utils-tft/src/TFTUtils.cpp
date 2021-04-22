#include "TFTUtils.hpp"

/******************************************************************************
*   D I S P L A Y  M E T H O D S
******************************************************************************/


void TFTUtils::displayInit() {
    pinMode(BUTTON_L, INPUT_PULLUP);
    pinMode(BUTTON_R, INPUT);
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
    ledcAttachPin(TFT_BL, pwmLedChannelTFT);
    setContrast(30);

    Serial.println("-->[TFT] display config ready.");
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
    Serial.println("-->[TFT] display welcome");
}

void TFTUtils::showMain() {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setSwapBytes(true);

    tft.setCursor(80, 204, 1);
    tft.println("BRIGHT:");

    tft.setCursor(80, 152, 2);
    tft.println("HEALTH:");

    tft.setTextColor(TFT_WHITE, lightblue);
    tft.setCursor(4, 152, 2);
    tft.println("TEMP:");

    tft.setCursor(4, 192, 2);
    tft.println("HUM: ");

    // tft.setTextColor(TFT_WHITE, TFT_BLACK);
    // tft.setFreeFont(&Orbitron_Medium_20);
    // tft.setCursor(6, 82);
    // tft.println("Berlin");

    tft.fillRect(68, 152, 1, 74, TFT_GREY);

    for (int i = 0; i < b + 1; i++)
        tft.fillRect(78 + (i * 7), 216, 3, 10, blue);

    Serial.println("-->[TFT] display welcome");
}

void TFTUtils::showProgress(unsigned int progress, unsigned int total) {
    Serial.println("-->[TFT] display progress");
    char output[12];
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.printf(output, "%03d%%", (progress / (total / 100)));
}

void TFTUtils::suspend() {
    delay(100);
    int r = digitalRead(TFT_BL);
    digitalWrite(TFT_BL, !r);
    delay(100);
    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);

    digitalWrite(ADC_EN, LOW);

    //After using light sleep, you need to disable timer wake, because here use external IO port to wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    esp_deep_sleep_disable_rom_logging();
    esp_deep_sleep_start();
}

void TFTUtils::checkButtons() {
    if (digitalRead(BUTTON_R) == 0) {
        if (press2 == 0) {
            press2 = 1;
            tft.fillRect(78, 216, 44, 12, TFT_BLACK);

            b++;
            if (b >= 5) suspend();
            // if (b >= 5) b = 0;

            for (int i = 0; i < b + 1; i++)
                tft.fillRect(78 + (i * 7), 216, 3, 10, blue);
            ledcWrite(pwmLedChannelTFT, backlight[b]);
        }
    } else
        press2 = 0;

    if (digitalRead(BUTTON_L) == 0) {
        if (press1 == 0) {
            press1 = 1;
            inv = !inv;
            tft.invertDisplay(inv);
        }
    } else
        press1 = 0;
}

void TFTUtils::welcomeAddMessage(String msg) {
    // Serial.println("-->[TFT] add message: "+msg);
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
    tft.setTextDatum(TC_DATUM);
    tft.fillRect(3, 8, 120, 30, TFT_BLACK);
    tft.setCursor(5, 34);
    tft.println(msg.c_str());
    tft.setTextFont(1);
    tft.setTextSize(0);
    tft.setCursor(100, 40);
    if (deviceType <= 3)
        tft.println("ug/m3");
    else
        tft.println("ppm");
}

void TFTUtils::displayBottomLine(String msg) {
    tft.fillRect(1, 224, 133, 8, TFT_BLACK);
    tft.setCursor(2, 232, 1);
    tft.println(msg);
}

void TFTUtils::displayEmoticonLabel(int cursor, String msg) {
    
}

void TFTUtils::displayEmoticonColor(uint32_t color, String msg) {
    tft.fillRect(78, 170, 56, 20, color);
    // tft.setFreeFont(&Orbitron_Medium_20);
    tft.setTextFont(1);
    // tft.setTextSize(0);
    tft.setCursor(81, 178);
    tft.println(msg);
}

void TFTUtils::displayBigEmoticon(String msg) {

}

void TFTUtils::displayBigLabel(int cursor, String msg) {

}

void TFTUtils::displaySensorAverage(int average, int deviceType) {
    if (average < 13) {
        displayEmoticonColor(TFT_GREEN, "GOOD");
    } else if (average < 36) {
        displayEmoticonColor(TFT_YELLOW, "MODERATE");
    } else if (average < 56) {
        displayEmoticonColor(TFT_ORANGE, "UNH SEN G");
    } else if (average < 151) {
        displayEmoticonColor(TFT_RED, "UNHEALTY");
    } else if (average < 251) {
        displayEmoticonColor(TFT_PURPLE, "VERY UNH");
    } else {
        displayEmoticonColor(TFT_BROWN, "HAZARDOUS");
    }
    char output[4];
    sprintf(output, "%04d", average);
    displayCenterBig(output, deviceType);
}

// TODO: separate this function, format/display
void TFTUtils::displaySensorData(int mainValue, int chargeLevel, float humi, float temp, int rssi, int deviceType) {
    tft.fillRect(1, 170, 64, 20, TFT_BLACK);
    tft.fillRect(1, 210, 64, 20, TFT_BLACK);
    
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setCursor(1, 187);
    tft.printf("%02.1f",temp);

    tft.setCursor(1, 227);
    tft.printf("%02d%%",(int)humi);

    // char output[50];
    // if (deviceType <= 4)
    //     sprintf(output, "%04d E%02d H%02f%% T%02f°C", mainValue, 0, humi, temp);
    // else
    //     sprintf(output, "%03d E%02d H%02f%% T%02f°C", mainValue, 0, humi, temp);

    // displayBottomLine(String(output));
    
    // tft.setFreeFont(&Orbitron_Medium_20);
    // tft.setCursor(2, 227);
    // sprintf(output, "%04d", mainValue);
    // tft.println(output);
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

}

void TFTUtils::pageEnd() {
    // u8g2.nextPage();
}

void TFTUtils::clearScreen() {
    tft.fillScreen(TFT_BLACK);
}

void TFTUtils::setContrast(uint32_t value) {
    ledcWrite(pwmLedChannelTFT, value);
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
