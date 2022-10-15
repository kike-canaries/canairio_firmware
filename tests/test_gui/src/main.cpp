/***
 * TESTING U8G2Lib
 * @hpsaturn 
 ***/

#include <Arduino.h>
#include <GUILib.hpp>

bool toggle;
GUIData data;

class MyGUIUserPreferencesCallbacks : public GUIUserPreferencesCallbacks {
    void onWifiMode(bool enable){
        Serial.println("-->[SETUP] onWifi changed: "+String(enable));
    };
    void onBrightness(int value){
        Serial.println("-->[SETUP] onBrightness changed: "+String(value));
    };
    void onColorsInverted(bool enable){
        Serial.println("-->[SETUP] onColorsInverted changed: "+String(enable));
    };
    void onSampleTime(int time){
        Serial.println("-->[SETUP] onSampleTime changed: "+String(time));
    };
    void onCalibrationReady(){
        Serial.println("-->[SETUP] onCalibrationReady");
    };
    void onPaxMode(bool enable){
        Serial.println("-->[SETUP] onPaxMode changed: "+String(enable));
    };
    void onUnitSelectionToggle() {
        Serial.println("-->[SETUP] onUnitSelectionToggle");
    };
    void onUnitSelectionConfirm() {
    };
    void onPowerOff(){
    };
};

void testSensorLiveIcon() {
    gui.displaySensorLiveIcon();
}

void testSendDataIcon() {
    gui.displayDataOnIcon();
}

void testSavePrefIcon() {
    gui.displayPreferenceSaveIcon();
}

void (*functionPtr[])() = {
    testSensorLiveIcon,
    testSendDataIcon,
    testSavePrefIcon
};

void testExtraWelcomeLines() {
    gui.welcomeAddMessage("InfluxDB test1..");  // test for multipage
    gui.welcomeAddMessage("InfluxDB test2..");
    gui.welcomeAddMessage("InfluxDB test3..");
    gui.welcomeAddMessage("InfluxDB test4..");
    gui.welcomeAddMessage("InfluxDB test5..");
    gui.welcomeAddMessage("Line test welcome 1");
    gui.welcomeAddMessage("Line test welcome 2");
    gui.welcomeAddMessage("Line test welcome 3");
    gui.welcomeAddMessage("Line test welcome 4");
}

void setup(void) {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n== INIT SETUP ==\n");

    gui.displayInit();
    gui.setCallbacks(new MyGUIUserPreferencesCallbacks());

    gui.showWelcome();
    // delay(500);
    gui.displayBottomLine("CanAirIOAF4");
    // delay(500);
    gui.welcomeAddMessage("Sensor ready..");
    // delay(500);
    gui.welcomeAddMessage("GATT server..");
    // delay(500);
    gui.welcomeAddMessage("WiFi with long SSID12345678");
    // delay(500);
    gui.welcomeAddMessage("GATT server.........ok");
    // testExtraWelcomeLines();
    gui.welcomeAddMessage("==SETUP READY==");

    randomSeed(A0);

    delay(500);
    gui.showMain();
    delay(100);
}

bool getBoolean() {
    return random(0, 2) == 1 ? true : false;
}

uint64_t count = 0;
int max_value = 1;


void loop(void) {

    if (count % 30 == 0 ) max_value = random (5,random(4,35));

    data.mainValue = random(1, max_value);
    data.minorValue = random(0, 99);
    data.unitName = "PAX";
    data.unitSymbol = "PAX";
    data.mainUnitId = 0;
    data.color = AQI_COLOR::AQI_PM;

    if (count % 5 == 0) gui.setSensorData(data);

    gui.setGUIStatusFlags(true, true, true);

    functionPtr[random(0, 3)]();  // Call a test function in random sequence

    // gui.showProgress(count++,1000);

    // gui.displayStatus(getBoolean(), true, getBoolean());

    count++;

    delay(500);
}
