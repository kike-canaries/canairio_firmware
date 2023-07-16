#include "TFTUtils.hpp"

/******************************************************************************
*   D I S P L A Y  M E T H O D S
******************************************************************************/

void guiTask(void* pvParameters) {
    Serial.println("-->[TGUI] starting task loop");
    while (1) {
        gui.pageStart();
        gui.checkButtons();
        gui.displayMainValues();
        gui.pageEnd();
        vTaskDelay(80 / portTICK_PERIOD_MS);
    }
}

void TFTUtils::setupGUITask() {
    taskGUIrunning = true;
    xTaskCreatePinnedToCore(
        guiTask,    /* Function to implement the task */
        "tempTask ", /* Name of the task */
        10000,        /* Stack size in words */
        NULL,        /* Task input parameter */
        5,           /* Priority of the task */
        &xHandle,    /* Task handle. */
        1);          /* Core where the task should run */
}

void TFTUtils::displayInit() {
    pinMode(BUTTON_L, INPUT_PULLUP);
    pinMode(BUTTON_R, INPUT);
    #ifdef M5STICKCPLUS
    M5.begin(true,true,false);       // Initialize M5Stack without serial messages
    M5.Beep.end();
    pinMode(36, INPUT);              // UART port alternative for this board
    gpio_pulldown_dis(GPIO_NUM_25);  // 36 and 25 pins share the same port
    gpio_pullup_dis(GPIO_NUM_25);    // https://docs.m5stack.com/en/core/m5stickc_plus
    #else
    tft.init();
    #endif
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    #ifndef M5STICKCPLUS
    ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
    ledcAttachPin(TFT_BL, pwmLedChannelTFT);
    #endif
    notifyBrightness();
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
    Serial.println("-->[TGUI] add message: "+msg);
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
    tft.setCursor(RCOLSTART, 204, 1);
    tft.println("BATT:");
    updateBatteryValue();

    tft.setCursor(RCOLSTART, 152, 2);
    tft.println("HEALTH:");

    tft.setTextColor(TFT_WHITE, lightblue);
    tft.setCursor(4, 152, 2);
    tft.println("TEMP:");

    tft.setCursor(4, 192, 2);
    tft.println("HUM: ");

    tft.fillRect(68, 152, 1, 74, TFT_GREY);

    // drawing battery container
    tft.drawRect(RCOLSTART-1, 215, 42, 12, TFT_GREY);
    tft.fillRect(RCOLSTART + 41, 217, 1, 8, TFT_GREY);
    tft.fillRect(RCOLSTART + 42, 219, 1, 4, TFT_GREY);
    
    state = 0;

    loadLastData();

    if(!taskGUIrunning) setupGUITask();           // init GUI thread

    Serial.println("-->[TGUI] displayed main screen");
}

void TFTUtils::showWindowBike(){
    holdR = 0;
    delay(100);
    showStatus();
    tft.setCursor(80, 204, 1);
    tft.println("BATT:");
    updateBatteryValue();

    tft.setCursor(80, 152, 2);
    tft.println("HEALTH:");

    tft.setTextColor(TFT_WHITE, lightblue);
    tft.setCursor(4, 152, 2);
    tft.println("KM:");

    tft.setCursor(4, 192, 2);
    tft.println("TIME: ");

    tft.fillRect(68, 152, 1, 74, TFT_GREY);

    state = 0;

    loadLastData();

    Serial.println("-->[TGUI] displayed bike screen");
}

void TFTUtils::showInfoWindow() {
    showStatus();
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.setTextFont(1);
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("DEVINFO", tft.width() / 2, 30);
    tft.drawLine(18,44,117,44,TFT_GREY);
    state = 7;
    refreshInfoWindow();
    Serial.println("-->[TGUI] displayed info screen");
}

void TFTUtils::refreshInfoWindow() {
    if (state != 7) return;
    tft.fillRect(0, 49, tft.width(), tft.height()-62, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextPadding(5);
    tft.setTextDatum(CR_DATUM);
    tft.setCursor(0, 50, 2);
    tft.println(_info);
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

    tft.setCursor(MARGINL, SSTART+PRESETH*4, 2);
    tft.println("CALIBRT:");

    tft.setCursor(MARGINL, SSTART+PRESETH*5, 2);
    tft.println("INFO:");

    tft.setCursor(MARVALL, SSTART+PRESETH*5, 2);
    tft.println(String(VERSION));

    updateInvertValue();
    updateWifiMode();
    updateSampleTime();
    loadBrightness();
    updateCalibrationField();

    tft.fillRect(68, SSTART, 1, 150, TFT_GREY);

    Serial.println("-->[TGUI] displayed setup screen");
}

void TFTUtils::refreshSetup() {
    int start = SSTART-3;
    if(state>1)tft.drawRect(0, start+((state-2)*PRESETH), 134, PRESETH, TFT_BLACK);
    tft.drawRect(0, start+((state-1)*PRESETH), 134, PRESETH, TFT_GREENYELLOW);
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
    if (state != 0) return;
    int batt_state = (int)_batteryCharge/20;
    String voltage = "" + String(_batteryVolts) + "v";
    displayBottomLine(voltage);
    tft.fillRect(RCOLSTART,216,40,10,TFT_BLACK);
    int color = _isCharging ? TFT_GREENYELLOW : blue;

    for (int i = 0; i < batt_state + 1 ; i++) {
        tft.fillRect(RCOLSTART + (i * 7), 217, 3, 8, i == 0 ? TFT_GREY : color);
    }
}

void TFTUtils::setWifiMode(bool enable){
    _wifi_enable = enable;
    updateWifiMode();
}

void TFTUtils::setPaxMode(bool enable){
    _pax_enable = enable;
    updateWifiMode();
}

void TFTUtils::notifyWifiMode(){
    if(_wifi_enable && !_pax_enable)  {
        _wifi_enable = !_wifi_enable;
        _pax_enable = !_pax_enable;
    } 
    else if (_pax_enable) _pax_enable = !_pax_enable;
    else _wifi_enable = !_wifi_enable;
    if(mGUICallBacks != nullptr) getInstance()->mGUICallBacks->onWifiMode(_wifi_enable);
    if(mGUICallBacks != nullptr) getInstance()->mGUICallBacks->onPaxMode(_pax_enable);
    updateWifiMode();
}

void TFTUtils::updateWifiMode(){
    if (state < 1) return;
    tft.fillRect(MARVALL, SSTART+PRESETH*2, 54, 13, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(MARVALL, SSTART+PRESETH*2, 2);
    if(_wifi_enable) tft.println("On");
    else if (_pax_enable) tft.println("PAX");
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

void TFTUtils::updateSampleTime() {
    if (state < 1) return;
    tft.fillRect(MARVALL, SSTART + PRESETH * 3, 54, 13, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(MARVALL, SSTART + PRESETH * 3, 2);
    tft.println("" + String(_sample_time) + "s");
}

void TFTUtils::updateCalibrationField(){
    static uint_fast64_t calibretts = 0;   // timestamp for GUI refresh
    if (state >= 1 && state < 6 && millis() - calibretts > 1000) {
        calibretts = millis();
        tft.fillRect(MARVALL, SSTART + PRESETH * 4, 54, 13, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(MARVALL, SSTART + PRESETH * 4, 2);
        if (_calibration_counter > 0){
            Serial.println("-->[TGUI] coundown to calibration: "+String(_calibration_counter));
            tft.println("" + String(_calibration_counter--) + "s");
        }
        else if (_calibration_counter == 0) {
            tft.println("READY");
            _calibration_counter=-1;
            if(mGUICallBacks != nullptr) getInstance()->mGUICallBacks->onCalibrationReady();
        }
        else if (_calibration_counter == -1)
            tft.println("READY");
        else 
            tft.println("START");
    }
}

void TFTUtils::startCalibration(){
    Serial.println("-->[TGUI] starting sensor calibration");
    _calibration_counter = 10;
}

void TFTUtils::toggleMain(){
    if(mGUICallBacks != nullptr && wstate==3) {
        getInstance()->mGUICallBacks->onUnitSelectionToggle(); // toggle secundary unit
    } else if(++wstate==2)wstate = 0;
    restoreMain();
}

void TFTUtils::restoreMain(){
    if(wstate==0)showMain();
    if(wstate==1)showWindowBike();
    if(wstate==3)showMain();
    if(_calibration_counter>=-1)_calibration_counter = -2;
}

void TFTUtils::loadLastData(){
    isNewData = true;
    drawBarGraph();
    // drawDualLineGraph();
    displaySensorAverage(_average);
    displayMainValues();
}

void TFTUtils::checkButtons() {
    if (digitalRead(BUTTON_R) == 0) {
        holdR++;
        if (pressR == 0) {
            pressR = 1;
            if(state==0)toggleMain();
            if(state==1)updateBrightness();
            if(state==2)invertScreen();
            if(state==3)notifyWifiMode();
            if(state==4)notifySampleTime();
            if(state==5)startCalibration();
            if(state==6)showInfoWindow();
        }
        if (holdR > 10) wstate = 3; // show secundary value for choose it
        if (holdR > 30) suspend();
    } else {
        holdR = 0;
        pressR = 0;
    }

    if (digitalRead(BUTTON_L) == 0) {
        holdL++;
        if (pressL == 0) {
            pressL = 1;
            if (state++ == 0 && wstate != 3) showSetup();
            else if(mGUICallBacks != nullptr && wstate==3) {
                if (_mainUnitId != _unit) resetBuffer(bufGraphMain);
                getInstance()->mGUICallBacks->onUnitSelectionConfirm(); // select secundary unit
                wstate = 0;
                restoreMain();
            }
            if (state >= 1) refreshSetup();
            if (state >= 7) restoreMain();
        }
        if(holdL > 10 && state >= 1) restoreMain();
        // if(holdL > 10 && state == 0) showWindowBike();
    } else {
        holdL = 0;
        pressL = 0;
    }
}

void TFTUtils::showProgress(unsigned int progress, unsigned int total) {
    suspendTaskGUI();
    if(progress == 0) showWelcome();
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setCursor(8, 103);
    tft.println("Updating:");

    tft.setFreeFont(&Orbitron_Light_32);
    tft.setTextDatum(TC_DATUM);
    tft.fillRect(6, 105, 120, 30, TFT_BLACK);
    tft.setCursor(8, 135);
    tft.printf("%03d%%",(progress / (total / 100)));
}

void TFTUtils::suspend() {
    showWelcome();
    welcomeAddMessage("");
    welcomeAddMessage("Shutting down in 3");
    delay(1000);
    int count = 2;
    while(digitalRead(BUTTON_R)==0 && count > 0){
        welcomeAddMessage("Shutting down in "+String(count--));
        delay(1000);
    } 
    if(digitalRead(BUTTON_R)!=0) {
        showMain();
        return;
    }
    welcomeAddMessage("");
    welcomeAddMessage("Suspending..");
    delay(2000);
    #ifdef M5STICKCPLUS
    M5.Axp.PowerOff();
    #elif ESP32S3

    #else
    int r = digitalRead(TFT_BL);
    digitalWrite(TFT_BL, !r);
    delay(10);
    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);
    if (mGUICallBacks != nullptr) getInstance()->mGUICallBacks->onPowerOff();   
    #endif
}

void TFTUtils::displayCenterBig(String msg) {
    tft.setFreeFont(&Orbitron_Light_32);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.drawString(msg.c_str(), tft.width() / 2, 36);
}

void TFTUtils::displayMainUnit(String uName, String uSymbol) {
    tft.setTextDatum(TR_DATUM);
    tft.setTextFont(1);
    tft.setTextSize(0);
    tft.drawString(uSymbol.c_str(),128,57);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(uName.c_str(),9,57);
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
    tft.fillRect(RCOLSTART-1, 170, 56, 20, color);
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
    int color =getAQIColor(average);
    displayEmoticonColor(aqicolors[color], aqilabels[color]);
}

void TFTUtils::displayMainHeader() {
    if (state != 0) return;
    tft.fillRect(2, 25, 130, 40, TFT_BLACK);
    if (wstate == 3 && toggle1s) return;
    char output[10];
    float value = 0.0;
    if (_unit == 0) value =  _mainValue;
    else value = _minorValue;
    
    uint16_t m_int_val = value;
    double m_tmp_val = value - m_int_val;
    bool isInt = !(m_tmp_val > 0);

    if (isInt) sprintf(output, "%04d", m_int_val);
    else sprintf(output, "%05.1f", value);

    displayCenterBig(output);
    displayMainUnit(_unit_name, _unit_symbol);
}

void TFTUtils::displayMainValues(){
    if (state == 0 && isNewData) {
        tft.fillRect(2, 25, 130, 40, TFT_BLACK);
        tft.fillRect(1, 170, 64, 20, TFT_BLACK);
        tft.fillRect(1, 210, 64, 20, TFT_BLACK);
        tft.setFreeFont(&Orbitron_Medium_20);

        if (wstate == 0 || wstate == 3) {
            tft.setCursor(1, 187);
            tft.printf("%02.1f", _temp);
            tft.setCursor(1, 227);
            tft.printf("%02d%%", (int)_humi);
            displayMainHeader();
        } 
        else {
            tft.setCursor(1, 187);
            tft.printf("%02.1f", _km);
            tft.setCursor(1, 227);
            tft.printf("%01d:%02d", _hours, _minutes);
            char output[20];
            sprintf(output, "%04.1f", _speed);
            displayCenterBig(output);
            displayMainUnit("Speed","KM/h");
        }
        
        drawBarGraph();
        // drawDualLineGraph();
        displaySensorAverage(_average);
        isNewData = false;
    }
}

// TODO: separate this function, format/display
void TFTUtils::setSensorData(GUIData data) {
    suspendTaskGUI();
    _colorType = data.color;
    _humi = data.humi;
    _temp = data.temp;
    _mainValue = data.mainValue;
    _mainUnitId = data.mainUnitId;
    _minorValue = data.minorValue;
    _unit_symbol = data.unitSymbol;
    _unit_name = data.unitName;
    _rssi = abs(data.rssi);
    if (_unit != data.onSelectionUnit) resetBuffer(bufGraphMinor);
    _unit = data.onSelectionUnit;
    bufGraphMain[MAX_X - 1] = _mainValue;
    bufGraphMinor[MAX_X - 1] = _minorValue;
    isNewData = true;
    resumeTaskGUI();
}

void TFTUtils::setGUIStatusFlags(bool wifiOn, bool bleOn, bool blePair) {
    static uint_fast64_t gui_status_ts = 0;  // timestamp for GUI refresh
    if ((millis() - gui_status_ts > 500)) {
        gui_status_ts = millis();
        suspendTaskGUI();
        _wifiOn = wifiOn;
        _bleOn = bleOn;
        _blePair = blePair;
        resumeTaskGUI();
    }
}

void TFTUtils::setInfoData(String info) {
    suspendTaskGUI();
    _info = info;
    resumeTaskGUI();
}

void TFTUtils::setBatteryStatus(float volts, int charge, bool isCharging) {
    suspendTaskGUI();
    _batteryVolts = volts;
    _batteryCharge = charge;
    _isCharging = isCharging;
    resumeTaskGUI();
}

void TFTUtils::setPowerSave() {
    // TODO: compatibility with OLED code
}

void TFTUtils::displayGUIStatusFlags() {
    static uint_fast64_t sensor_status_ts = 0;   // timestamp for GUI refresh
    if ((millis() - sensor_status_ts > 1000)) { 
        sensor_status_ts = millis();

        tft.fillRect(0, 0, 135, 18, TFT_BLACK);

        if (_bleOn && _blePair)
            drawBluetoothIcon();
        if (_wifiOn){
            if (_rssi < 60) drawWifiHighIcon();
            else if (_rssi < 70) drawWifiMidIcon();
            else drawWifiLowIcon();
        }

        if (sensorLive) drawFanIcon();
        if (dataOn) drawDataIcon();
        if (preferenceSave) drawPreferenceSaveIcon();

        if (dataOn) dataOn = false;                              // reset trigger for publish data ok.
        if (preferenceSave) preferenceSave = false;              // reset trigger for save preference ok.
        if (sensorLive && _live_ticks++>1) sensorLive = false;   // reset fan animation

    }
}

uint32_t TFTUtils::getAQIColor(uint32_t value) {
    if (_colorType == AQI_COLOR::AQI_PM) {

        if (value <= 13)       return 0;
        else if (value <= 35)  return 1;
        else if (value <= 55)  return 2;
        else if (value <= 150) return 3;
        else if (value <= 250) return 4;
        else                   return 5;

    } 
    else if (_colorType == AQI_COLOR::AQI_CO2) {
        if (value <= 600)       return 0;
        else if (value <= 800)  return 1;
        else if (value <= 1000) return 2;
        else if (value <= 1500) return 3;
        else if (value <= 2000) return 4;
        else                    return 5;

    } 
     if (_colorType == AQI_COLOR::AQI_CO) {
//Conversion factor at 20ºC and 1013 hPa, 1ppm=1165 mg/m3 and 1 mg/m3=0,858 ppm
// 1 ppm is equivalent to 1.16mg/m3 for a gas with molecular weight=28.01, Pressure=1013.25 mbar, Temperature=20C ****** TO CHECK *******
// 1 ppm is equivalent to 1.25mg/Nm3 for a gas with molecular weight=28.01, Pressure=1013.25 mbar, Temperature=0C

        if (value <= 6)        return 0;
        else if (value <= 8)   return 1;
        else if (value <= 11)  return 2;
        else if (value <= 17)  return 3;
        else if (value > 17)   return 4;
        else                   return 5;

    } 
    else if (_colorType == AQI_COLOR::AQI_NH3) {
//1 ppm is equivalent to 0.71mg/m3 for a gas with molecular weight=17.031, Pressure=1013.25 mbar, Temperature=20C
// 1 ppm is equivalent to 0.76mg/Nm3 for a gas with molecular weight=17.031, Pressure=1013.25 mbar, Temperature=0C
        
        if (value <= 7)         return 0;
        else if (value <= 10)   return 1;
        else if (value <= 14)   return 2;
        else if (value <= 21)   return 3;
        else if (value > 21)    return 4;
        else                    return 5;
    }
    else return 0;
}

void TFTUtils::drawBarGraph() {
    double mainFactor = getMultiplicator(bufGraphMain);
    double minorFactor = getMultiplicator(bufGraphMinor);
    int lenMain, lenMinor;
    tft.fillRect(0, 149 - MAX_Y, MAX_X, MAX_Y, TFT_BLACK);
    tft.drawLine(0, 150 - MAX_Y, MAX_X - 1, 150 - MAX_Y,TFT_GREY);
    _average = 0; 
    for (int i = 0; i < MAX_X; i++) {
        lenMain = bufGraphMain[i] * mainFactor;    // main value
        lenMinor = bufGraphMinor[i] * minorFactor; // secondary value
        _average = bufGraphMain[i] + _average;     // average value only for main value

        int color = TFT_WHITE;
        if (_unit == _mainUnitId || _unit == 0) {
            if (_colorType > AQI_COLOR::AQI_NONE) color = aqicolors[getAQIColor(bufGraphMain[i])];
            tft.drawLine(i, 150, i, 150 - (lenMain > MAX_Y ? MAX_Y : lenMain),color);
        }
        else {
            if (_colorType > AQI_COLOR::AQI_NONE) color = aqicolors[getAQIColor(bufGraphMinor[i])];
            tft.drawLine(i, 150, i, 150 - (lenMinor > MAX_Y ? MAX_Y : lenMinor),color);
        }
        
        if (i < MAX_X - 1) {
            bufGraphMain[i] = bufGraphMain[i + 1];
            bufGraphMinor[i] = bufGraphMinor[i + 1];
        }
    }

    _average = _average / MAX_X;
}

void TFTUtils::drawLineGraph() {
    double multiplicator = getMultiplicator(bufGraphMinor);
    int len;
    tft.fillRect(0, 149 - MAX_Y, MAX_X, MAX_Y, TFT_BLACK);
    tft.drawLine(0, 150 - MAX_Y, MAX_X - 1, 150 - MAX_Y,TFT_GREY);
    for (int i = 0; i < MAX_X; i++) {
        len = bufGraphMinor[i] * multiplicator;
        tft.drawLine(i, 150, i, 150 - (len > MAX_Y ? MAX_Y : len),TFT_WHITE);
        if (i < MAX_X - 1) bufGraphMinor[i] = bufGraphMinor[i + 1];
    }
}

void TFTUtils::drawDualLineGraph() {
    double mainFactor = getMultiplicator(bufGraphMain);
    double minorFactor = getMultiplicator(bufGraphMinor);
    int lenMain, lenMinor;
    tft.fillRect(0, 149 - MAX_Y, MAX_X, MAX_Y, TFT_BLACK);
    tft.drawLine(0, 150 - MAX_Y, MAX_X - 1, 150 - MAX_Y,TFT_GREY);
    _average = 0; 
    for (int i = 0; i < MAX_X; i++) {
        lenMain = bufGraphMain[i] * mainFactor;    // main value
        lenMinor = bufGraphMinor[i] * minorFactor; // secondary value
        _average = bufGraphMain[i] + _average;     // average value only for main value

        int color = TFT_WHITE;
        int y = 0;
        if (_unit == 0 ) {
            if (_colorType > AQI_COLOR::AQI_NONE) color = aqicolors[getAQIColor(bufGraphMain[i])];
            y = (lenMain > MAX_Y ? MAX_Y : lenMain);
            tft.drawLine(i, 152 - y, i, 150 - y, color);
        }
        else {
            if (_colorType > AQI_COLOR::AQI_NONE) color = aqicolors[getAQIColor(bufGraphMinor[i])];
            y = (lenMinor > MAX_Y ? MAX_Y : lenMinor);
            tft.drawLine(i, 152 - y, i, 150 - y, color);
        }
        
        if (i < MAX_X - 1) {
            bufGraphMain[i] = bufGraphMain[i + 1];
            bufGraphMinor[i] = bufGraphMinor[i + 1];
        }
    }

    _average = _average / MAX_X;
}

void TFTUtils::resetBuffer(uint32_t *buf) {
    for (int i = 0; i < MAX_X; i++) buf[i] = 0;
}

double TFTUtils::getMultiplicator(uint32_t * buf) {
  uint32_t maxVal = 1;
  for (int i = 0; i < MAX_X; i++) {
    if (buf[i] > maxVal) maxVal = buf[i];
  }
  if (maxVal > MAX_Y) return (double)MAX_Y / (double)maxVal;
  else if (maxVal < MAX_Y / 20) return 20;
  else if (maxVal < MAX_Y / 15) return 15;
  else if (maxVal < MAX_Y / 10) return 10;
  else if (maxVal < MAX_Y / 5)  return 5;
  else if (maxVal < MAX_Y / 3)  return 3;
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

void TFTUtils::drawPreferenceSaveIcon () {
    tft.drawBitmap(83, 1, iconSave, 12, 16, TFT_BLACK, TFT_GREEN);
}

void TFTUtils::pageStart() {
    // slow interactions 
    static uint_fast64_t loopts = 0;   // slow GUI refresh
    if (millis() - loopts > 2000) {  
        loopts = millis();
        refreshInfoWindow();
        updateBatteryValue();
    }
    static uint_fast64_t loop500ms = 0; // 500ms refresh GUI
    if (millis() - loop500ms > 500) { 
        loop500ms = millis();
        toggle1s = !toggle1s;
        if (wstate == 3 ) displayMainHeader();
    }
    /// fast interactions (80ms)
    if(sensorLive) drawFanIcon();
    updateCalibrationField();
    displayGUIStatusFlags();
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
    #ifdef M5STICKCPLUS
    M5.Axp.ScreenBreath(brightness);
    #else
    ledcWrite(pwmLedChannelTFT, brightness);
    #endif
}

void TFTUtils::setCallbacks(GUIUserPreferencesCallbacks* pCallBacks){
    mGUICallBacks = pCallBacks;
}

void TFTUtils::setTrackValues(float speed, float distance){
    suspendTaskGUI();
    _speed = speed;
    _km = distance;
    resumeTaskGUI();
}

void TFTUtils::setTrackTime(int h, int m, int s){
    suspendTaskGUI();
    _hours = h;
    _minutes = m;
    _seconds = s;
    resumeTaskGUI();
}

void TFTUtils::setEmoticons(bool enable) {
}

void TFTUtils::flipVertical(bool enable){
  if(enable) tft.setRotation(2);
  else tft.setRotation(0);
}

void TFTUtils::suspendTaskGUI(){
    if(taskGUIrunning) vTaskSuspend(xHandle);
}

void TFTUtils::resumeTaskGUI(){
    if(taskGUIrunning) vTaskResume(xHandle);
}

TFTUtils* TFTUtils::getInstance() {
	return this;
}

void TFTUtils::loop(){
    // OLED GUI compatibility
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
