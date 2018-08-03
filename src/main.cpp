
/**
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018
 * @brief HPMA115S0 sensor on ESP32 with bluetooth GATT notify server
 * @license GPL3
 */

#include <hpma115S0.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <U8g2lib.h>
// Display via i2c for WeMOS OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
// Debugging flag
bool DEBUG = true;
// firmware version from git rev-list command
String VERSION_CODE = "rev";
int VCODE = SRC_REV;
//Create an instance of hardware serial
HardwareSerial hpmaSerial(1);
// Create an instance of the hpma115S0 library
#define SAMPLING_RATE 5000
HPMA115S0 hpma115S0(hpmaSerial);
unsigned int count = 0;
unsigned int pm2_5, pm10;

// BLE vars
BLEServer* pServer = NULL;
BLECharacteristic* pCharactPM25 = NULL;
BLECharacteristic* pCharactPM10 = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "c8d1d262-861f-4082-947e-f383a259aaf3"
#define CHARAC_PM25_UUID    "b0f332a8-a5aa-4f3f-bb43-f99e7791ae01"
#define CHARAC_PM10_UUID    "b0f332a8-a5aa-4f3f-bb43-f99e7791ae02"

void displayInit(){
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setContrast(255);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  Serial.println("-->OLED ready");
}

void showWelcome(){
  u8g2.clearBuffer();
  String version = "ESP32 HPMA115 ("+String(VERSION_CODE+VCODE)+")";
  u8g2.drawStr(0, 0,version.c_str());
  u8g2.drawLine(0, 11, 128, 11);
  u8g2.sendBuffer();
  Serial.println("-->Welcome screen ready\n");
  delay(1000);
}

void displayOnBuffer(String msg){
  u8g2.drawStr(0, 16, msg.c_str());
  u8g2.sendBuffer();
}

void sensorInit(){
  Serial.print("-->Starting hpma115S0..");
  hpma115S0.Init();
  hpma115S0.StartParticleMeasurement();
  hpma115S0.DisableAutoSend();
  delay(20);
  Serial.println("done");
}

/**
* PM2.5 and PM10 read and visualization functions
*/
void sensorRead(){
  if (hpma115S0.ReadParticleMeasurement(&pm2_5, &pm10)) {
    if(count<1000)count++;
    else count=0;
    Serial.print(String(count)+"\tPm2.5:\t" + String(pm2_5) + " ug/m3\t" );
    Serial.println("Pm10:\t" + String(pm10) + " ug/m3" );
    // String display = String(count)+" P25: " + String(pm2_5) + " | P10: " + String(pm10);
    char output[20];
    if(pm2_5<1000&&pm10<1000){
      sprintf(output,"%03d P25:%03d P10:%03d",count,pm2_5,pm10);
      // String display = String(count)+" P25: " + String(pm2_5) + " | P10: " + String(pm10);
      displayOnBuffer(String(output));
    }
  }
  else{
    Serial.println("Warnning: hpma115S0 cant not read!");
    displayOnBuffer(String(count)+" E: read error!");
  }
}

String sensorGetRead25(){
  return String("{")+"\"P25\":"+String(pm2_5)+"}"; // max supported 20 chars
}

String sensorGetRead10(){
  return String("{")+"\"P10\":"+String(pm10)+"}"; // max supported 20 chars
}

void resetVars(){
  count=0;
}

class MyServerCallbacks: public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
      Serial.println("[BLE] onConnect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("[BLE] onDisconnect");
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
  Serial.println("-->BLE ready. (Waiting a client to notify)");
}

void bleLoop(){
  // notify changed value
  if (deviceConnected) {
    sensorRead();
    pCharactPM25->setValue(sensorGetRead25().c_str());
    pCharactPM25->notify();
    delay(SAMPLING_RATE); // bluetooth stack will go into congestion, if too many packets are sent
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("[BLE] start advertising");
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

void setup() {
  Serial.println("\nINIT SETUP:\n");
  Serial.begin(9600);
  Serial.println("\n-->DebugConsole ready");
  displayInit();
  hpmaSerial.begin(9600,SERIAL_8N1,13,15);
  Serial.println("-->HardwareSerial ready");
  delay(10);
  sensorInit();
  bleServerInit();
  showWelcome();
}

void loop() {
  bleLoop();
}
