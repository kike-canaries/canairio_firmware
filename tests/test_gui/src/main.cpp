/***
 * TESTING U8G2Lib
 * @hpsaturn 
 ***/

#include <Arduino.h>
#include <GUILib.hpp>

unsigned int tcount = 0;
bool toggle;

void testSensorLiveIcon(){
  gui.displaySensorLiveIcon();
}

void testSendDataIcon(){
  gui.displayDataOnIcon();
}

void testSavePrefIcon(){
  gui.displayPreferenceSaveIcon();
}

void (*functionPtr[])() = { 
  testSensorLiveIcon,
  testSendDataIcon,
  testSavePrefIcon
};

void setup(void) {
  Serial.begin(115200);
  Serial.println("\n== INIT SETUP ==\n");
  Serial.println("-->[SETUP] console ready");
  gui.displayInit();
  gui.showWelcome();
  gui.welcomeAddMessage("Sensor ready..");
  gui.welcomeAddMessage("GATT server..");
  gui.welcomeAddMessage("WiFi test..");
  gui.welcomeAddMessage("InfluxDB test1..");  // test for multipage
  gui.welcomeAddMessage("InfluxDB test2..");
  gui.welcomeAddMessage("InfluxDB test3..");
  gui.welcomeAddMessage("InfluxDB test4..");
  gui.welcomeAddMessage("InfluxDB test5..");
  gui.welcomeAddMessage("Line test welcome 1");
  gui.welcomeAddMessage("Line test welcome 2");
  gui.welcomeAddMessage("Line test welcome 3");
  gui.welcomeAddMessage("Line test welcome 4");
  gui.welcomeAddMessage("==SETUP READY==");

  randomSeed(A0);

  delay(1000);
}

bool getBoolean(){
  return random(0,2) == 1 ? true : false;
}

void loop(void) {

  int rnd = random(0, 3);

  gui.pageStart();

  gui.displaySensorAverage(random(0, 999),1);

  gui.displaySensorData(120, 230, 15, 3.5, 12.3, random(0,99));

  gui.displayStatus(getBoolean(),true,getBoolean());
  
  functionPtr[rnd]();       // Call a test function in random sequence

  gui.pageEnd();

  delay(1000);
}

