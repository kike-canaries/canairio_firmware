
/**
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018
 * @brief HPMA115S0 sensor on ESP32 with bluetooth GATT notify server
 * @license GPL3
 */

#include <Arduino.h>
#include <InfluxArduino.hpp>
#include <ArduinoJson.h>
#include <vector>
#include <numeric>
#include <hpma115S0.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <U8g2lib.h>
#include "WiFi.h"

using namespace std;

// Firmware version from git rev-list command
String VERSION_CODE = "rev";
#ifdef SRC_REV
int VCODE = SRC_REV;
#else
int VCODE = 0;
#endif
/******************************************************************************
* S E T U P  B O A R D
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
// HPMA115S0 sensor config
#ifdef WEMOSOLED
#define HPMA_RX 13   // config for Wemos board
#define HPMA_TX 15
#elif HELTEC
#define HPMA_RX 13  // config for Heltec board
#define HPMA_TX 12
#else
#define HPMA_RX 17  // config for D1MIN1 board
#define HPMA_TX 16
#endif
HardwareSerial hpmaSerial(1);
HPMA115S0 hpma115S0(hpmaSerial);
String txtMsg = "";
vector<int> v25;      // for avarage
vector<int> v10;      // for avarage
unsigned int mcount, ecount = 0;
int interval = 5000;

// Bluetooth variables
BLEServer* pServer = NULL;
BLECharacteristic* pCharactData = NULL;
BLECharacteristic* pCharactConfig = NULL;
BLECharacteristic* pCharactAuth = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
#define SERVICE_UUID        "c8d1d262-861f-4082-947e-f383a259aaf3"
#define CHARAC_DATA_UUID    "b0f332a8-a5aa-4f3f-bb43-f99e7791ae01"
#define CHARAC_CONFIG_UUID  "b0f332a8-a5aa-4f3f-bb43-f99e7791ae03"
#define CHARAC_AUTH_UUID    "b0f332a8-a5aa-4f3f-bb43-f99e7791ae04"

// InfluxDB variables:
InfluxArduino influx;
// connection database stuff that needs configuring
const char WIFI_NAME[] = "xxxx";
const char WIFI_PASS[] = "xxxx";
const char INFLUX_DATABASE[] = "mydb";
const char INFLUX_IP[] = "aireciudadano.servehttp.com";
const char INFLUX_USER[] = ""; //username if authorization is enabled.
const char INFLUX_PASS[] = ""; //password for if authorization is enabled.
const char INFLUX_MEASUREMENT[] = "PM2.5_EST6_noHum_524";

unsigned long DELAY_TIME_US = 10 * 1000 * 1000; //how frequently to send data, in microseconds
unsigned long countINF = 0; //a variable that we gradually increase in the loop
#define LED 2

/******************************************************************************
*   D I S P L A Y  M E T H O D S
******************************************************************************/
void displayInit(){
  Serial.println("-->[OLED] setup display..");
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setContrast(255);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  u8g2.setFontMode(0);
  Serial.println("-->[OLED] display ready.");
}

void showWelcome(){
  u8g2.clearBuffer();
#ifdef D1MINI
  u8g2.drawStr(0, 0, "CanAirIO");
  String version = "("+String(VERSION_CODE+VCODE)+")";
  u8g2.drawStr(0, 11,version.c_str());
  u8g2.drawLine(0, 22, 128, 22);
#else
  String version = "CanAirIO ("+String(VERSION_CODE+VCODE)+")";
  u8g2.drawStr(0, 0,version.c_str());
  u8g2.drawLine(0, 11, 128, 11);
#endif
  u8g2.sendBuffer();
  Serial.println("-->[OLED] welcome screen ready\n");
  delay(1000);
}

void displayBottomLine(String msg){
#ifndef D1MINI
  u8g2.setCursor(0, 16);
  u8g2.print(msg.c_str());
  u8g2.sendBuffer();
#else
  u8g2.setCursor(0, 40);
  u8g2.print(msg.c_str());
  u8g2.sendBuffer();
#endif
}

void displayCenterBig(String msg){
#ifdef D1MINI
  u8g2.setCursor(0,0);
  u8g2.setFont(u8g2_font_inb27_mn);
#else
  u8g2.setCursor(73,40);
  u8g2.setFont(u8g2_font_freedoomr25_mn);
#endif
  u8g2.print(msg.c_str());
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.sendBuffer();
}

void displaySensorError(String msg){
  u8g2.clearBuffer();
#ifdef D1MINI
  u8g2.setCursor(0, 40);
  u8g2.print(msg.c_str());
  u8g2.sendBuffer();
#else
  u8g2.setCursor(0, 32);
  u8g2.print(msg.c_str());
  u8g2.sendBuffer();
#endif
}

void displayAvarage(int avarage){
  char output[4];
  sprintf(output, "%03d", avarage);
  displayCenterBig(output);
}

/******************************************************************************
*   S E N S O R  M E T H O D S
******************************************************************************/

// [DEPRECATED] sensorConfig:
// The next method is only if sensor was config without autosend.

void sensorConfig(){
  Serial.println("-->[HPMA] configuration hpma115S0 sensor..");
  hpmaSerial.begin(9600,SERIAL_8N1,HPMA_RX,HPMA_TX);
  hpma115S0.Init();
  delay(100);
  hpma115S0.EnableAutoSend();
  delay(100);
  hpma115S0.StartParticleMeasurement();
  delay(100);
  Serial.println("-->[HPMA] sensor configured.");
}

void sensorInit(){
  Serial.println("-->[HPMA] starting hpma115S0 sensor..");
  delay(100);
  hpmaSerial.begin(9600,SERIAL_8N1,HPMA_RX,HPMA_TX);
  Serial.println("-->[HPMA] init hpma serial ready..");
  Serial.println("-->[HPMA] sensor ready.");
  delay(100);
}

void wrongDataState(){
  Serial.println("wrong data!");
  char output[22];
  if(ecount>999)ecount=0;
  sprintf(output,"%04d E:%03d",mcount,ecount++);
  displaySensorError(output);
  txtMsg="";
  hpmaSerial.end();
  sensorInit();
  delay(1000);
}

/**
* PM2.5 and PM10 read and visualization
*/

void hpmaSerialRead(){
  Serial.print("-->[HPMA] read.");
  while (txtMsg.length() < 32) {
    while (hpmaSerial.available() > 0) {
      char inChar = hpmaSerial.read();
      txtMsg += inChar;
      Serial.print(".");
    }
  }
  if(mcount<9999)mcount++;
  else mcount=0;
  if (txtMsg[0] == 66) {
    if (txtMsg[1] == 77) {
      Serial.print("done");
      unsigned int pm25 = txtMsg[6] * 256 + byte(txtMsg[7]);
      unsigned int pm10 = txtMsg[8] * 256 + byte(txtMsg[9]);
      txtMsg="";
      if(pm25<1000&&pm10<1000){
        char output[22];
        v25.push_back(pm25); // for PM25 avarage
        v10.push_back(pm10);
#ifdef D1MINI
        sprintf(output,"%04d P:%03d",mcount,pm25);
#else
        sprintf(output,"%04d P25:%03d P10:%03d",mcount,pm25,pm10);
#endif
        Serial.println(" --> "+String(output)+" E:"+String(ecount));
        displayBottomLine(String(output));
      }
      else wrongDataState();
    }
    else wrongDataState();
  }
  else wrongDataState();
}

int getPM25Avarage(){
  int pm25_avarage = accumulate( v25.begin(), v25.end(), 0.0)/v25.size();
  v25.clear();
  return pm25_avarage; 
}

int getPM10Avarage(){
  int pm10_avarage = accumulate( v10.begin(), v10.end(), 0.0)/v10.size();
  v10.clear();
  return pm10_avarage; 
}

String getFormatData(int pm25, int pm10){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["P25"] = pm25;
  root["P10"] = pm10;
  String json;
  root.printTo(json);
  return json;
}

void hpmaSerialLoop(){
  delay(1000);
  hpmaSerialRead();
}

/******************************************************************************
*   C O N F I G  M E T H O D S
******************************************************************************/

String getConfigData(){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["mode"]      = 0;
  root["stime"]     = 5;
  interval = 5*1000;  // Mockup for now
  String output;
  root.printTo(output);
  return output;
}

/******************************************************************************
*   B L U E T O O T H  M E T H O D S
******************************************************************************/
class MyServerCallbacks: public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
      Serial.println("-->[BLE] onConnect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("-->[BLE] onDisconnect");
      deviceConnected = false;
    };
}; // BLEServerCallbacks

class MyConfigCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        Serial.print("-->[BLE CONFIG] ");
        Serial.println(value.c_str());
      }
    }
};

class MyAuthCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        Serial.print("-->[BLE AUTH] ");
        Serial.println(value.c_str());
      }
    }
};

void bleServerInit(){
  // Create the BLE Device
  BLEDevice::init("ESP32_HPMA115S0");
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic for PM 2.5
  pCharactData = pService->createCharacteristic(
      CHARAC_DATA_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  // Create a BLE Characteristic for Sensor mode: STATIC/MOVIL
  pCharactConfig = pService->createCharacteristic(
      CHARAC_CONFIG_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  // Create a BLE Characteristic for Chredentials
  pCharactAuth = pService->createCharacteristic(
      CHARAC_AUTH_UUID,
      BLECharacteristic::PROPERTY_WRITE_NR
  );
  // Create a Data Descriptor (for notifications)
  pCharactData->addDescriptor(new BLE2902());
  // Setting Config callback
  pCharactConfig->setCallbacks(new MyConfigCallbacks());
  // Getting saved config data
  pCharactConfig->setValue(getConfigData().c_str());
  // Setting Auth callback
  pCharactAuth->setCallbacks(new MyAuthCallbacks());
  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("-->[BLE] GATT server ready. (Waiting for client)");
}

void bleLoop(){
  // notify changed value
  if (deviceConnected && v25.size() > 4) {  // ~5 sec aprox
    int pm25 = getPM25Avarage();
    int pm10 = getPM10Avarage();
    pCharactData->setValue(getFormatData(pm25,pm10).c_str());
    pCharactData->notify();
    displayAvarage(pm25);
  }
  else if (v25.size() > 4) {
    displayAvarage(getPM25Avarage());
    getPM10Avarage(); // clear vector (possible overflow)
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("-->[BLE] start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

/******************************************************************************
*   I N F L U X D B   M E T H O D S
******************************************************************************/

void influxDbInit() {
  influx.configure(INFLUX_DATABASE, INFLUX_IP); //third argument (port number) defaults to 8086
  // influx.authorize(INFLUX_USER,INFLUX_PASS); //if you have set the Influxdb .conf variable auth-enabled to true, uncomment this
  // influx.addCertificate(ROOT_CERT); //uncomment if you have generated a CA cert and copied it into InfluxCert.hpp
  Serial.print("Using HTTPS: ");
  Serial.println(influx.isSecure()); //will be true if you've added the InfluxCert.hpp file.
}

bool influxDbWrite(char *tags, char *fields) {
  return influx.write(INFLUX_MEASUREMENT, tags, fields);
}

/******************************************************************************
*   W I F I   M E T H O D S
******************************************************************************/

void wifiConfigInit() {
  pinMode(LED,OUTPUT);

  WiFi.begin(WIFI_NAME, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");
}

/******************************************************************************
*  M A I N
******************************************************************************/

void setup() {
  Serial.begin(115200);
  Serial.println("\n== INIT SETUP ==\n");
  Serial.println("-->[SETUP] console ready");
  displayInit();
  sensorInit();
  bleServerInit();
  showWelcome();
  Serial.println("-->[SETUP] setup ready");
}

void loop() {
  hpmaSerialLoop();
  bleLoop();
}
