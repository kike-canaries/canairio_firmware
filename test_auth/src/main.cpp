
/**
 * Authentication tests
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018 - 2019
 * @brief CanAirIO API tests
 * @license GPL3
 */

#include <Arduino.h>
#include <Wire.h>
#include <InfluxArduino.hpp>
#include <CanAirIoApi.hpp>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <GUIUtils.hpp>
#include <Preferences.h>
#include <bitset>

/******************************************************************************
* S E T U P  B O A R D   A N D  F I E L D S
* ---------------------
* please select board on platformio.ini file
******************************************************************************/

#ifdef WEMOSOLED // display via i2c for WeMOS OLED board
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
#elif HELTEC // display via i2c for Heltec board
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 15, 4, 16);
#else       // display via i2c for D1MINI board
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE,U8X8_PIN_NONE,U8X8_PIN_NONE);
#endif

using namespace std;

vector<unsigned int> v25;      // for average
vector<unsigned int> v10;      // for average
unsigned int apm25 = 0;        // last PM2.5 average
unsigned int apm10 = 0;        // last PM10 average
int stime = 5;                 // sample time (send data each 5 sec)
double lat,lon;                // Coordinates
float alt, spd;                // Altitude and speed

// WiFi fields
#define WIFI_RETRY_CONNECTION    20
String ssid, pass;
bool dataSendToggle;
bool wifiEnable, wifiOn;
bool isNewWifi;
uint64_t chipid;

// CanAirIO API fields
CanAirIoApi api(false);

// InfluxDB fields
#define IFX_RETRY_CONNECTION   5
InfluxArduino influx;
String ifxdb, ifxip, ifxuser, ifxpassw, ifxid, ifxtg, ifusr, ifpss, ifcer;
uint16_t ifxpt;
bool isNewIfxdbConfig;

// GUI fields
#define LED 2
GUIUtils gui;

// Config Settings
Preferences preferences;
const char app_name[] = "canairio";

void reboot();
void wifiStop();

#include "status.h"
#include "settings.h"

/******************************************************************************
*   W I F I   M E T H O D S
******************************************************************************/

bool wifiCheck(){
  wifiOn = WiFi.isConnected();
  if(wifiOn)statusOn(bit_wan);  // TODO: We need validate internet connection
  else {
    statusOff(bit_cloud);
    statusOff(bit_wan);
  }
  return wifiOn;
}

void wifiConnect(const char* ssid, const char* pass) {
  Serial.print("-->[WIFI] Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, pass);
  int wifi_retry = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_retry++ < WIFI_RETRY_CONNECTION) {
    Serial.print(".");
    delay(250);
  }
  if(wifiCheck()){
    isNewWifi=false;  // flag for config via BLE
    Serial.println("done\n-->[WIFI] connected!");
  }
  else{
    Serial.println("fail!\n-->[E][WIFI] disconnected!");
    setErrorCode(ecode_wifi_fail);
  }
}

void wifiInit(){
  if(wifiEnable && ssid.length() > 0 && pass.length() > 0) {
    wifiConnect(ssid.c_str(), pass.c_str());
  }
}

void wifiStop(){
  if(wifiOn){
    Serial.println("-->[WIFI] Disconnecting..");
    WiFi.disconnect(true);
    wifiCheck();
  }
}

void wifiRestart(){
  wifiStop();
  wifiInit();
}

void wifiLoop(){
  if(v25.size()==0 && wifiEnable && ssid.length()>0 && !wifiCheck()) {
    wifiConnect(ssid.c_str(), pass.c_str());
    // influxDbReconnect();
  }
}
 
/******************************************************************************
*   C A N A I R I O  P U B L I S H   M E T H O D S
******************************************************************************/
void apiInit(){
  Serial.println("-->[API] Starting..");
  char id[13];
  sprintf(id,"%04X%08X",(uint16_t)(chipid >> 32),(uint32_t)chipid);
  Serial.println("-->[API] configure id:"+String(id));
  api.configure(id, "points/save/", "canairio.herokuapp.com"); //third argument (port number) defaults to 8086
  //api.authorize(ifusr.c_str(),ifpss.c_str());
  Serial.println("-->[API] authorize..");
  api.authorize("canairio","canairio_password");
  delay(1000);
}

void apiLoop() {
  if (wifiOn) {
    Serial.print("-->[API] write..");
    if(api.write(0,apm25,apm10,0,0,lat,lon,alt,spd,stime)){
       Serial.println("done!");
       gui.welcomeAddMessage("test: pass!");
       while(1);
    }
    else{
       Serial.println("failed!");
       Serial.println("-->[API] response: "+String(api.getResponse()));
       gui.welcomeAddMessage("test:fail! "+String(api.getResponse()));
       while(1);
    }
  }
}

/******************************************************************************
*  M A I N
******************************************************************************/
void printDeviceId(){
  Serial.printf("-->[INFO] ESP32MAC: %04X", (uint16_t)(chipid >> 32)); //print High 2 bytes
  Serial.printf("%08X\n", (uint32_t)chipid);                           //print Low 4bytes.
}

void reboot() {
  delay(100);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  gui.displayInit(u8g2);
  gui.showWelcome();
  Serial.println("\n== INIT SETUP ==\n");
  chipid=ESP.getEfuseMac(); 
  printDeviceId();
  Serial.println("-->[SETUP] serial ready.");
  configInit();
  if(ssid.length()>0) gui.welcomeAddMessage("WiFi:"+ssid);
  else gui.welcomeAddMessage("WiFi radio test..");
  wifiInit();
  gui.welcomeAddMessage("CanAirIO API..");
  apiInit();
  pinMode(LED,OUTPUT);
  gui.welcomeAddMessage("==SETUP READY==");
  delay(500);
  gui.welcomeAddMessage("testing API..");
}
 
void loop() {
  delay(5000);
  apiLoop();
}