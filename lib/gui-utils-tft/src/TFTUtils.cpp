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
    notifyBrightness();

    setupBattery();           // init battery ADC.

    Serial.println("-->[TGUI] display config ready.");
}

void TFTUtils::showWelcome() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setCursor(2,20);
    tft.print("CanAirIO ");
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(101,226);
    tft.println(getFirmwareVersionCode().c_str());
    tft.drawLine(0,24,135,24,TFT_GREEN);
    lastDrawedLine = 32;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(1);
    Serial.println("-->[TGUI] displayed welcome screen");
}

void TFTUtils::welcomeAddMessage(String msg) {
    // Serial.println("-->[TGUI] add message: "+msg);
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

void TFTUtils::showStatus() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(0);
    tft.setCursor(110,231);
    tft.println(getFirmwareVersionCode().c_str());
    tft.drawLine(0,19,135,19,TFT_GREEN);
    tft.setSwapBytes(true);
}

void TFTUtils::showMain() {
    showStatus();
    tft.setCursor(80, 204, 1);
    tft.println("BATT:");
    // updateBatteryValue();

    tft.setCursor(80, 152, 2);
    tft.println("HEALTH:");

    tft.setTextColor(TFT_WHITE, lightblue);
    tft.setCursor(4, 152, 2);
    tft.println("TEMP:");

    tft.setCursor(4, 192, 2);
    tft.println("HUM: ");

    tft.fillRect(68, 152, 1, 74, TFT_GREY);

    Serial.println("-->[TGUI] displayed main screen");
}

void TFTUtils::showWindowBike(){
    showStatus();
    tft.setCursor(80, 204, 1);
    tft.println("BATT:");
    // updateBatteryValue();

    tft.setCursor(80, 152, 2);
    tft.println("HEALTH:");

    tft.setTextColor(TFT_WHITE, lightblue);
    tft.setCursor(4, 152, 2);
    tft.println("KM:");

    tft.setCursor(4, 192, 2);
    tft.println("TIME: ");

    tft.fillRect(68, 152, 1, 74, TFT_GREY);

    loadLastData();

    Serial.println("-->[TGUI] displayed bike window");
}

void TFTUtils::showSetup() {
    showStatus();
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("SETUP", tft.width() / 2, 30);

    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawLine(18,44,117,44,TFT_GREY);

    tft.setTextColor(TFT_WHITE, lightblue);
    tft.setCursor(MARGINL, SSTART, 2);
    tft.println("BRIGHT:");
    
    tft.setCursor(MARGINL, SSTART+PRESETH, 2);
    tft.println("COLORS:");

    tft.setCursor(MARGINL, SSTART+PRESETH*2, 2);
    tft.println("WiFi:");

    tft.setCursor(MARGINL, SSTART+PRESETH*3, 2);
    tft.println("STIME:");

    updateInvertValue();
    updateWifiMode();
    updateSampleTime();
    loadBrightness();

    tft.fillRect(68, SSTART, 1, 150, TFT_GREY);

    Serial.println("-->[TGUI] displayed setup screen");
}

void TFTUtils::refreshSetup() {
    int start = SSTART-3;
    if(state>1)tft.drawRect(0, start+((state-2)*PRESETH), 134, PRESETH, TFT_BLACK);
    tft.drawRect(0, start+((state-1)*PRESETH), 134, PRESETH, TFT_GREY);
}

void TFTUtils::loadBrightness() {
    for (int i = 0; i < 5; i++) {
        if (backlight[i] == brightness) b = i;
    }
    for (int i = 0; i < b + 1; i++) {
        tft.fillRect(MARVALL + (i * 7), SSTART + 2, 3, 10, blue);
    }
}

void TFTUtils::updateBrightness() {
    tft.fillRect(MARVALL, SSTART+1, 44, 11, TFT_BLACK);
    b++;
    if (b >= 5) b = 0;
    for (int i = 0; i < b + 1; i++)
        tft.fillRect(MARVALL + (i * 7), SSTART+2, 3, 10, blue);
    brightness = backlight[b];
    notifyBrightness();
    if(mGUICallBacks != nullptr) getInstance()->mGUICallBacks->onBrightness(brightness);
}

void TFTUtils::invertScreen(){
    inv = !inv;
    tft.invertDisplay(inv);
    updateInvertValue();
    if(mGUICallBacks != nullptr) getInstance()->mGUICallBacks->onColorsInverted(inv);
}

void TFTUtils::updateInvertValue(){
    tft.fillRect(MARVALL, SSTART+PRESETH, 54, 13, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(MARVALL, SSTART+PRESETH, 2);
    if(inv) tft.println("normal");
    else tft.println("inverted");
}

void TFTUtils::updateBatteryValue(){
    float volts = battGetVoltage();
    int state = (int)battCalcPercentage(volts)/20;

    // String voltage = "" + String(volts) + "v";
    // displayBottomLine(voltage);
    tft.fillRect(78,216,44,10,TFT_BLACK);

    for (int i = 0; i < state + 1; i++) {
        tft.fillRect(78 + (i * 7), 216, 3, 10, blue);
    }
}

void TFTUtils::setWifiMode(bool enable){
    _wifi_enable = enable;
    updateWifiMode();
}

void TFTUtils::notifyWifiMode(){
    _wifi_enable = !_wifi_enable;
    updateWifiMode();
    if(mGUICallBacks != nullptr) getInstance()->mGUICallBacks->onWifiMode(_wifi_enable);
}

void TFTUtils::updateWifiMode(){
    tft.fillRect(MARVALL, SSTART+PRESETH*2, 54, 13, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(MARVALL, SSTART+PRESETH*2, 2);
    if(_wifi_enable) tft.println("On");
    else tft.println("Off");
}

void TFTUtils::notifySampleTime(){
    if(++st>=5) st = 0;
    _sample_time = sampleTime[st];
    updateSampleTime();
    if(mGUICallBacks != nullptr) getInstance()->mGUICallBacks->onSampleTime(_sample_time);
}

void TFTUtils::setSampleTime(int time){
    _sample_time = time;
    updateSampleTime();
}

void TFTUtils::updateSampleTime(){
    tft.fillRect(MARVALL, SSTART+PRESETH*3, 54, 13, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(MARVALL, SSTART+PRESETH*3, 2);
    tft.println(""+String(_sample_time)+"s");
}

void TFTUtils::toggleWindow(){
    wstate++;
    if(wstate==1)showWindowBike();
    if(wstate==2)restoreMain();
}

void TFTUtils::restoreMain(){
    showMain();
    delay(10);
    state = 0;
    wstate = 0;
    loadLastData();
    Serial.println("-->[TGUI] displayed restored main");
}

void TFTUtils::loadLastData(){
    drawBarGraph(_deviceType);
    displaySensorAverage(_average);
    displaySensorData(_mainValue, 0, _humi, _temp, _rssi, _deviceType);
}

void TFTUtils::checkButtons() {
    if (digitalRead(BUTTON_R) == 0) {
        if (press2 == 0) {
            press2 = 1;
            if(state==0)toggleWindow();
            if(state==1)updateBrightness();
            if(state==2)invertScreen();
            if(state==3)notifyWifiMode();
            if(state==4)notifySampleTime();
        }
    } else
        press2 = 0;

    if (digitalRead(BUTTON_L) == 0) {
        if (press1 == 0) {
            if (digitalRead(BUTTON_R)==0) suspend();
            press1 = 1;
            if(state++==0)showSetup();
            if(state>=1)refreshSetup();
            if(state==5)restoreMain();
        }
    } else
        press1 = 0;
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
    showWelcome();
    welcomeAddMessage("Shutting down..");
    while(digitalRead(BUTTON_R)!=0) delay(10);
    delay(2000);
    int r = digitalRead(TFT_BL);
    digitalWrite(TFT_BL, !r);
    delay(10);
    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);
    digitalWrite(ADC_EN, LOW);
    delay(10);
    //Disable timer wake, because here use external IO port to wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    esp_deep_sleep_disable_rom_logging();
    esp_deep_sleep_start();
}

void TFTUtils::displayCenterBig(String msg) {
    tft.setFreeFont(&Orbitron_Light_32);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.fillRect(2, 25, 130, 32, TFT_BLACK);
    tft.drawString(msg.c_str(), tft.width() / 2, 36);
}

void TFTUtils::displayMainUnit(String unit) {
    tft.setTextDatum(TR_DATUM);
    tft.setTextFont(1);
    tft.setTextSize(0);
    tft.drawString(unit.c_str(),123,57);
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

void TFTUtils::displaySensorAverage(int average) {
    if (state == 0) {
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
    }
}

// TODO: separate this function, format/display
void TFTUtils::displaySensorData(int mainValue, int chargeLevel, float humi, float temp, int rssi, int deviceType) {
    if (state == 0) {

        char output[5];
        tft.fillRect(1, 170, 64, 20, TFT_BLACK);
        tft.fillRect(1, 210, 64, 20, TFT_BLACK);
        tft.setFreeFont(&Orbitron_Medium_20);

        if (wstate == 0) {
            
            tft.setCursor(1, 187);
            tft.printf("%02.1f", temp);

            tft.setCursor(1, 227);
            tft.printf("%02d%%", (int)humi);

            _humi = humi;
            _temp = temp;
            _mainValue = mainValue;

            sprintf(output, "%04d", mainValue);
            displayCenterBig(output);

            if (deviceType <= 3)
                displayMainUnit("PM2.5");
            else
                displayMainUnit("PPM");
        }
        else {
            tft.setCursor(1, 187);
            tft.printf("%02.1f", km);

            tft.setCursor(1, 227);
            tft.printf("%01d:%02d", hours, minutes);

            sprintf(output, "%03.1f", speed);
            displayCenterBig(output);
            displayMainUnit("KM/h");
        }

        _rssi = abs(rssi);
        pkts[MAX_X - 1] = mainValue;

        drawBarGraph(deviceType);
        displaySensorAverage(_average);
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

uint32_t TFTUtils::getAQIColor(uint32_t value, int deviceType) {
    if (deviceType <= 3) {

        if (value <= 13)       return TFT_GREEN;
        else if (value <= 35)  return TFT_YELLOW;
        else if (value <= 55)  return TFT_ORANGE;
        else if (value <= 150) return TFT_RED;
        else if (value <= 250) return TFT_PURPLE;
        else                   return TFT_BROWN;

    } else {

        if (value <= 600)       return TFT_GREEN;
        else if (value <= 800)  return TFT_YELLOW;
        else if (value <= 1000) return TFT_ORANGE;
        else if (value <= 1500) return TFT_RED;
        else if (value <= 2000) return TFT_PURPLE;
        else                    return TFT_BROWN;
    }
}

void TFTUtils::drawBarGraph(int deviceType) {
    double multiplicator = getMultiplicator();
    int len;
    tft.fillRect(0, 149 - MAX_Y, MAX_X, MAX_Y, TFT_BLACK);
    tft.drawLine(0, 150 - MAX_Y, MAX_X - 1, 150 - MAX_Y,TFT_GREY);
    _average = 0; 
    for (int i = 0; i < MAX_X; i++) {
        len = pkts[i] * multiplicator;
        _average = pkts[i] + _average;
        int color = getAQIColor(pkts[i],deviceType);
        tft.drawLine(i, 150, i, 150 - (len > MAX_Y ? MAX_Y : len),color);
        if (i < MAX_X - 1) pkts[i] = pkts[i + 1];
    }

    _average = _average / MAX_X;
}

double TFTUtils::getMultiplicator() {
  uint32_t maxVal = 1;
  for (int i = 0; i < MAX_X; i++) {
    if (pkts[i] > maxVal) maxVal = pkts[i];
  }
  if (maxVal > MAX_Y) return (double)MAX_Y / (double)maxVal;
  else if (maxVal < MAX_Y / 5) return 4;
  else if (maxVal < MAX_Y / 4) return 3;
  else if (maxVal < MAX_Y / 3) return 2;
  else return 1;
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
    tft.drawBitmap(122, 1, iconBluetoothPaired, 12, 16, TFT_BLACK, TFT_WHITE);
}

void TFTUtils::drawWifiHighIcon () {
    tft.drawBitmap(109, 1, iconWifiHigh, 12, 16, TFT_BLACK, TFT_WHITE);
}

void TFTUtils::drawWifiMidIcon () {
    tft.drawBitmap(109, 1, iconWifiMid, 12, 16, TFT_BLACK, TFT_WHITE);
}

void TFTUtils::drawWifiLowIcon () {
    tft.drawBitmap(109, 1, iconWifiLow, 12, 16, TFT_BLACK, TFT_WHITE);
}

void TFTUtils::drawFanIcon () {
    tft.drawBitmap(0, 1, fanState ? iconFanState0 : iconFanState1, 12, 16, TFT_BLACK, TFT_GREEN);
    fanState = !fanState;
}

void TFTUtils::drawDataIcon () {
    tft.drawBitmap(96, 1, iconArrows, 12, 16, TFT_BLACK, TFT_WHITE);
}

void TFTUtils::pageStart() {
    // fast interactions (80ms)
    checkButtons();
    if(sensorLive) drawFanIcon();
    // slow interactions
    static uint_fast64_t loopts = 0;   // timestamp for GUI refresh
    if (state == 0 && (millis() - loopts > 5000)) {  
        loopts = millis();
        updateBatteryValue();
    }
}

void TFTUtils::pageEnd() {

}

void TFTUtils::clearScreen() {
    tft.fillScreen(TFT_BLACK);
}

void TFTUtils::setBrightness(uint32_t value) {
    brightness = value;
}

void TFTUtils::notifyBrightness() {
    ledcWrite(pwmLedChannelTFT, brightness);
}

void TFTUtils::setCallbacks(GUIUserPreferencesCallbacks* pCallBacks){
    mGUICallBacks = pCallBacks;
}

TFTUtils* TFTUtils::getInstance() {
	return this;
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
