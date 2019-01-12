
/**
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018
 * @brief HPMA115S0 sensor on ESP32 with bluetooth GATT notify server
 * @license GPL3
 */

#include <vector>
#include <numeric>
#include <hpma115S0.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <U8g2lib.h>
#include "WiFi.h"

///////////
#include "InfluxArduino.hpp"
#include "InfluxCert.hpp"

#include <Wire.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
///////////

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
vector<long> v;      // for avarage
unsigned int pm2_5, pm10, count, ecount = 0;


///////////
Adafruit_AM2320 am2320 = Adafruit_AM2320();

String TramaSD = "";
byte SDflag = 0;

byte Num = 0;
unsigned long PMtotal = 0;
unsigned int PMs = 0;
float PMdiv = 0;

String dataMessage = "";
float humidity = 0;
float temperature = 0;
unsigned int HUMint=0; //!!!
unsigned int pm25promedio=0;

InfluxArduino influx;
//connection/ database stuff that needs configuring
const char WIFI_NAME[] = "BelkinTX";
const char WIFI_PASS[] = "apt413sago16";
const char INFLUX_DATABASE[] = "mydb";
const char INFLUX_IP[] = "192.168.2.4";
const char INFLUX_USER[] = ""; //username if authorization is enabled.
const char INFLUX_PASS[] = ""; //password for if authorization is enabled.
const char INFLUX_MEASUREMENT[] = "pm25"; //measurement name for the database. (in practice, you can use several, this example just uses the one)

unsigned long DELAY_TIME_US = 10 * 1000 * 1000; //how frequently to send data, in microseconds
unsigned long countINF = 0; //a variable that we gradually increase in the loop
#define LED 2

///////////

// Bluetooth variables
BLEServer* pServer = NULL;
BLECharacteristic* pCharactPM25 = NULL;
BLECharacteristic* pCharactPM10 = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
#define SERVICE_UUID        "c8d1d262-861f-4082-947e-f383a259aaf3"
#define CHARAC_PM25_UUID    "b0f332a8-a5aa-4f3f-bb43-f99e7791ae01"
#define CHARAC_PM10_UUID    "b0f332a8-a5aa-4f3f-bb43-f99e7791ae02"

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

void displaySensorData1(String msg){
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

void displaySensorData2(String msg){
#ifndef D1MINI
  u8g2.setCursor(0, 16);
  u8g2.print(msg.c_str());
  u8g2.sendBuffer();

#else
  u8g2.setCursor(0, 32);
  u8g2.print(msg.c_str());
  u8g2.sendBuffer();
#endif
}

void displayLastPM25(String msg){
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
  if(ecount>9999)ecount=0;
  sprintf(output,"     E:%03d",ecount++);   ////////////
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
  if(count<99999)count++;
  else count=0;
  if (txtMsg[0] == 66) {
    if (txtMsg[1] == 77) {
      Serial.print("done");
      pm2_5 = txtMsg[6] * 256 + byte(txtMsg[7]);
      pm10 = txtMsg[8] * 256 + byte(txtMsg[9]);
      txtMsg="";
      if(pm2_5<1000&&pm10<1000){
        char output[22];
        v.push_back(pm2_5); // for avarage

////////
    PMtotal = PMtotal + pm2_5;
////////

#ifdef D1MINI
        sprintf(output,"%05d P:%03d",count,pm2_5);
#else
        sprintf(output,"%04d P25:%03d P10:%03d",count,pm2_5,pm10);
#endif
        Serial.println(" --> "+String(output)+" E:"+String(ecount));
        displaySensorData2(String(output));
      }
      else wrongDataState();
    }
    else wrongDataState();
  }
  else wrongDataState();
}

String sensorGetRead25Avarage(){
  int pm2_5_avarage = accumulate( v.begin(), v.end(), 0.0)/v.size();
  char output[4];
  sprintf(output,"%03d",pm2_5_avarage);
  displayLastPM25(output);

///////////

  pm2_5_avarage = pm25promedio;

  sprintf(output,"%05d  %03d",count,pm2_5);
  displaySensorData2(String(output));
  sprintf(output,"H=%02d P=%03d",HUMint,PMs);
  displaySensorData1(String(output));

///////////


  v.clear();
  return String("{")+"\"P25\":"+String(pm2_5_avarage)+"}"; // max supported 20 chars
}

void resetVars(){
  count=0;
}

void hpmaSerialLoop(){
  if(deviceConnected)hpmaSerialRead();
}

/******************************************************************************
*   HUMIDITY  M E T H O D S
******************************************************************************/

void AM2320Read() {

          dataMessage = "Trama " + String(count) + "; " + String(pm2_5) + "; " + String(humidity) + "\r\n";
          Serial.print("Save data: ");
          Serial.print(dataMessage);
          TramaSD+= dataMessage;

     if (count%5==0) {
            humidity = am2320.readHumidity();
            temperature = am2320.readTemperature();
            Serial.print("AM2320 Hum: ");
            Serial.print(humidity);
            Serial.print(" %       ");
            Serial.print("Temp: ");
            Serial.print(temperature);
            Serial.println(" °C");
            if (count==0) {
              count++;
            }
            PMdiv = PMtotal/count;
            PMs = round (PMdiv); //promedio
        //    HUMint = (int) humidity;
            HUMint = round (humidity);
     }


     if ((humidity>94) & (humidity<101)){
      // mirar que colocar aqui
   }
}

void AM2320Loop(){
  if(deviceConnected)AM2320Read();
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

void bleServerInit(){
  // Create the BLE Device
  BLEDevice::init("ESP32_HPMA115S0");
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic for PM 2.5
  pCharactPM25 = pService->createCharacteristic(
                      CHARAC_PM25_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  // Create a BLE Descriptor
  pCharactPM25->addDescriptor(new BLE2902());
  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("-->[BLE] GATT server ready. (Waiting a client to notify)");
}

void bleLoop(){
  // notify changed value
  if (deviceConnected && v.size() > 4) {  // ~5 sec aprox
    pCharactPM25->setValue(sensorGetRead25Avarage().c_str());
    pCharactPM25->notify();

//////////////
  char tags[16];
  char fields[128];

 sprintf(tags, "read_ok=true");
 sprintf(fields,"count=%d,pm25promedio=%d",count,pm25promedio);

 bool writeSuccessful = influx.write(INFLUX_MEASUREMENT, tags, fields);

 digitalWrite(LED, HIGH);

 if (!writeSuccessful) {
   Serial.print("error: ");
   Serial.println(influx.getResponse());
 }
 else {
   delay(50);
   digitalWrite(LED, LOW);
 }
/////////////

  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("-->[BLE] start advertising");
    oldDeviceConnected = deviceConnected;
    showWelcome();
    resetVars();
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

/******************************************************************************
*   WIFI  M E T H O D S
******************************************************************************/
/*
void wifiSmartConfigInit() {
  //Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  //Wait for SmartConfig packet from mobile
  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone())
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("SmartConfig received.");

  //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected.");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

*/

//***********************************************************
void wifiConfigInit() {
    pinMode(LED,OUTPUT);

    WiFi.begin(WIFI_NAME, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("WiFi connected!");
    influx.configure(INFLUX_DATABASE,INFLUX_IP); //third argument (port number) defaults to 8086
    // influx.authorize(INFLUX_USER,INFLUX_PASS); //if you have set the Influxdb .conf variable auth-enabled to true, uncomment this
    // influx.addCertificate(ROOT_CERT); //uncomment if you have generated a CA cert and copied it into InfluxCert.hpp
    Serial.print("Using HTTPS: ");
    Serial.println(influx.isSecure()); //will be true if you've added the InfluxCert.hpp file.

}

//***********************************************************



/******************************************************************************
*  M A I N
******************************************************************************/

void setup() {
  Serial.begin(115200);
  Serial.println("\n== INIT SETUP ==\n");
  Serial.println("-->[SETUP] console ready");
  displayInit();
  sensorInit();

//////////

  am2320.begin();
  Serial.println("-->[AM2320] sensor ready.");
  //wifiConfigInit();

/////////

  bleServerInit();
  wifiConfigInit(); ///////////////
  //wifiSmartConfigInit();
  showWelcome();
  Serial.println("-->[SETUP] setup ready");
}

void loop() {
  hpmaSerialLoop();
  AM2320Loop();    //directo
  bleLoop();
}
