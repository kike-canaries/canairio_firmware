
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

// firmware version from git rev-list command
String VERSION_CODE = "rev";
#ifdef SRC_REV
int VCODE = SRC_REV;
#else
int VCODE = 0;
#endif

// Config ESP32 board
#ifdef WEMOS_OLED
// display via i2c for WeMOS OLED board
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
#else
// display via i2c for Heltec board
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 15, 4, 16);
#endif

// HPMA115S0 sensor connection config
#ifdef WEMOS_OLED
#define HPMA_RX 13   // config for Wemos board
#define HPMA_TX 15
#else
#define HPMA_RX 13  // config for Heltec board
#define HPMA_TX 12
#endif

using namespace std;

HardwareSerial hpmaSerial(1);
HPMA115S0 hpma115S0(hpmaSerial);
String txtMsg = "";
unsigned int pm2_5, pm10, count = 0;
vector<int> v;

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
  String version = "ESP32 HPMA115 ("+String(VERSION_CODE+VCODE)+")";
  u8g2.drawStr(0, 0,version.c_str());
  u8g2.drawLine(0, 11, 128, 11);
  u8g2.sendBuffer();
  Serial.println("-->[OLED] welcome screen ready\n");
  delay(1000);
}

void displaySensorData(String msg){
  u8g2.setCursor(0, 16);
  u8g2.print(msg.c_str());
  u8g2.sendBuffer();
}

void displaySensorError(String msg){
  u8g2.setCursor(0, 26);
  u8g2.print(msg.c_str());
  u8g2.sendBuffer();
}

/******************************************************************************
*   S E N S O R  M E T H O D S
******************************************************************************/
/**
*  TODO: the next method is only for first time
*  the idea is via bluetooth config
*/
void sensorConfig(){
  hpma115S0.Init();
  delay(100);
  hpma115S0.EnableAutoSend();
  delay(100);
  hpma115S0.StartParticleMeasurement();
  delay(1000);
}

void sensorInit(){
  Serial.println("-->[HPMA] starting hpma115S0 sensor..");
  delay(100);
  hpmaSerial.begin(9600,SERIAL_8N1,HPMA_RX,HPMA_TX);
  Serial.println("-->[HPMA] init hpma serial ready..");
  // sensorConfig();
  Serial.println("-->[HPMA] sensor ready.");
  delay(100);
}

void wrongDataState(){
  Serial.println("wrong data!");
  char output[22];
  sprintf(output,"last error: %03d",count);
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
  if(count<999)count++;
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
        sprintf(output,"%03d P25:%03d P10:%03d",count,pm2_5,pm10);
        Serial.println("-->[HPMA] "+String(output));
        displaySensorData(String(output));
      }
      else wrongDataState();
    }
    else wrongDataState();
  }
  else wrongDataState();
}

String sensorGetRead25Avarage(){
  int pm2_5_avarage = accumulate( v.begin(), v.end(), 0.0)/v.size();
  v.clear();
  return String("{")+"\"P25\":"+String(pm2_5_avarage)+"}"; // max supported 20 chars
}

String sensorGetRead10(){
  return String("{")+"\"P10\":"+String(pm10)+"}"; // max supported 20 chars
}

void resetVars(){
  count=0;
}

void hpmaSerialLoop(){
  if(deviceConnected)hpmaSerialRead();
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

  // TODO: to research possible issue with two characteristics,
  // notitications are accumalated and lag when sending it

  // Create a BLE Characteristic for PM 10
  //pCharactPM10 = pService->createCharacteristic(
  //                   CHARAC_PM10_UUID,
  //                   BLECharacteristic::PROPERTY_READ   |
  //                    BLECharacteristic::PROPERTY_NOTIFY
  //                 );

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
