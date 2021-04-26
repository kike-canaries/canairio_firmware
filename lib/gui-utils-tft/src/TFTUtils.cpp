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
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setCursor(2,20);
    Serial.println("-->[TFT] font H: "+String(tft.fontHeight()));
    tft.print("CanAirIO ");
    tft.setTextFont(2);
    Serial.println("-->[TFT] font H: "+String(tft.fontHeight()));
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(101,226);
    tft.println(getFirmwareVersionCode().c_str());
    tft.drawLine(0,24,135,24,TFT_GREEN);
    lastDrawedLine = 32;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(1);
    Serial.println("-->[TFT] display welcome");
}

void TFTUtils::welcomeAddMessage(String msg) {
    // Serial.println("-->[TFT] add message: "+msg);
    tft.setTextFont(1);
    tft.setCursor(5, lastDrawedLine);
    tft.println(msg.substring(0,21).c_str());
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

void TFTUtils::showMain() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(0);
    tft.setCursor(110,231);
    tft.println(getFirmwareVersionCode().c_str());
    tft.drawLine(0,19,135,19,TFT_GREEN);
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

    tft.fillRect(68, 152, 1, 74, TFT_GREY);

    for (int i = 0; i < b + 1; i++)
        tft.fillRect(78 + (i * 7), 216, 3, 10, blue);

    Serial.println("-->[TFT] display welcome");
}

void TFTUtils::showProgress(unsigned int progress, unsigned int total) {
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setCursor(8, 103);
    tft.println("Update:");

    tft.setFreeFont(&Orbitron_Light_32);
    tft.setTextDatum(TC_DATUM);
    tft.fillRect(6, 105, 120, 30, TFT_BLACK);
    tft.setCursor(8, 135);
    tft.printf("%03d%%",(progress / (total / 100)));
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

void TFTUtils::displayCenterBig(String msg, int deviceType) {
    tft.setFreeFont(&Orbitron_Light_32);
    tft.setTextDatum(TC_DATUM);
    tft.fillRect(3, 25, 120, 30, TFT_BLACK);
    tft.setCursor(5, 51);
    tft.println(msg.c_str());
    tft.setTextFont(1);
    tft.setTextSize(0);
    tft.setCursor(100, 57);
    if (deviceType <= 3)
        tft.println("ug/m3");
    else
        tft.println("ppm");
}

void TFTUtils::displayBottomLine(String msg) {
    tft.setTextFont(1);
    tft.fillRect(1, 230, 99, 8, TFT_BLACK);
    tft.setCursor(2, 232, 1);
    tft.println(msg.substring(0,16).c_str());
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
    static uint_fast64_t sensor_avarage_ts = 0;   // timestamp for GUI refresh
    if ((millis() - sensor_avarage_ts > 1000)) {  
        sensor_avarage_ts = millis();
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
}

// TODO: separate this function, format/display
void TFTUtils::displaySensorData(int mainValue, int chargeLevel, float humi, float temp, int rssi, int deviceType) {
    static uint_fast64_t sensor_data_ts = 0;   // timestamp for GUI refresh
    if ((millis() - sensor_data_ts > 1000)) {  // it should be minor than sensor loop
        sensor_data_ts = millis();
        tft.fillRect(1, 170, 64, 20, TFT_BLACK);
        tft.fillRect(1, 210, 64, 20, TFT_BLACK);

        tft.setFreeFont(&Orbitron_Medium_20);
        tft.setCursor(1, 187);
        tft.printf("%02.1f", temp);

        tft.setCursor(1, 227);
        tft.printf("%02d%%", (int)humi);

        _rssi = rssi;

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
}

void TFTUtils::displayStatus(bool wifiOn, bool bleOn, bool blePair) {
    static uint_fast64_t sensor_status_ts = 0;   // timestamp for GUI refresh
    if ((millis() - sensor_status_ts > 1000)) { 
        sensor_status_ts = millis();

        tft.fillRect(0, 0, 135, 18, TFT_BLACK);

        if (bleOn && blePair)
            drawBluetoothIcon();
        if (wifiOn){
            if (_rssi < 60) drawWifiHighIcon();
            else if (_rssi < 70) drawWifiMidIcon();
            else drawWifiLowIcon();
        }

        if (sensorLive) drawFanIcon();
        if (dataOn) drawDataIcon();
        // if (preferenceSave);

        if (dataOn) dataOn = false;                              // reset trigger for publish data ok.
        if (preferenceSave) preferenceSave = false;              // reset trigger for save preference ok.
        if (sensorLive && _live_ticks++>1) sensorLive = false;   // reset fan animation
    }
}

/// enable trigger for show data ok icon, one time.
void TFTUtils::displayDataOnIcon() {
    dataOn = true;
}

/// enable trigger for sensor live icon, one time.
void TFTUtils::displaySensorLiveIcon() {
    sensorLive = true;
    _live_ticks = 0;
}

/// enable trigger for save preference ok, one time.
void TFTUtils::displayPreferenceSaveIcon() {
    preferenceSave = true;
}

void TFTUtils::drawBluetoothIcon () {
    tft.drawBitmap(122, 1, iconBluetoothPaired, 12, 16, TFT_BLACK, TFT_BLUE);
}

void TFTUtils::drawWifiHighIcon () {
    tft.drawBitmap(109, 1, iconWifiHigh, 12, 16, TFT_BLACK, TFT_BLUE);
}

void TFTUtils::drawWifiMidIcon () {
    tft.drawBitmap(109, 1, iconWifiMid, 12, 16, TFT_BLACK, TFT_BLUE);
}

void TFTUtils::drawWifiLowIcon () {
    tft.drawBitmap(109, 1, iconWifiLow, 12, 16, TFT_BLACK, TFT_BLUE);
}

void TFTUtils::drawFanIcon () {
    tft.drawBitmap(0, 1, fanState ? iconFanState0 : iconFanState1, 12, 16, TFT_BLACK, TFT_GREEN);
    fanState = !fanState;
}

void TFTUtils::drawDataIcon () {
    tft.drawBitmap(96, 1, iconArrows, 12, 16, TFT_BLACK, TFT_LIGHTGREY);
}

void TFTUtils::pageStart() {
    // fast interactions (80ms)
    checkButtons();
    if(sensorLive) drawFanIcon();
}

void TFTUtils::pageEnd() {

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
