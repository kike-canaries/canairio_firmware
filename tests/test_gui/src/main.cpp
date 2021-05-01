/***
 * TESTING U8G2Lib
 * @hpsaturn 
 ***/

#include <Arduino.h>
#include <GUILib.hpp>

bool toggle;

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
    delay(500);
    Serial.println("\n== INIT SETUP ==\n");
    Serial.println("-->[SETUP] console ready");
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
}

bool getBoolean() {
    return random(0, 2) == 1 ? true : false;
}

uint64_t count = 20;
uint16_t total = 1000;
int max_value = random(0,150);

void loop(void) {

    gui.pageStart();

    if (count % 300 == 0 ) max_value = random (3,120);

    if (count % 30 == 0) gui.displaySensorData(random(0,max_value), 230,random(0, 99), random(0, 800)/25.0, random(50, 90), 1);

    if (count % 15 == 0) functionPtr[random(0, 3)]();  // Call a test function in random sequence

    // gui.showProgress(count++,1000);

    // gui.displayStatus(getBoolean(), true, getBoolean());
    
    gui.displayStatus(true, true, true);
    gui.checkButtons();
    gui.pageEnd();

    delay(80);

    count++;

}
