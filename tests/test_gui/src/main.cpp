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
    void onCalibrationReady(){
        Serial.println("-->[SETUP] onCalibrationReady");
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

void guiTask(void* pvParameters) {
    Serial.println("-->[Setup] GUI task loop");
    while (1) {

        gui.pageStart();
        gui.displayMainValues();
        gui.displayStatus(true, true, true);
        gui.pageEnd();

        delay(80);
    }
}

void setupGUITask() {
    xTaskCreatePinnedToCore(
        guiTask,    /* Function to implement the task */
        "tempTask ", /* Name of the task */
        10000,        /* Stack size in words */
        NULL,        /* Task input parameter */
        5,           /* Priority of the task */
        NULL,        /* Task handle. */
        1);          /* Core where the task should run */
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
    setupGUITask();
}

bool getBoolean() {
    return random(0, 2) == 1 ? true : false;
}

uint64_t count = 0;
int max_value = 1;


void loop(void) {

    if (count % 30 == 0 ) max_value = random (5,random(4,35));

    if (count % 5 == 0) gui.setSensorData(random(1,max_value), 230,random(0, 99), random(0, 800)/25.0, random(50, 90), 4);

    functionPtr[random(0, 3)]();  // Call a test function in random sequence

    // gui.showProgress(count++,1000);

    // gui.displayStatus(getBoolean(), true, getBoolean());

    count++;

    delay(500);
}
