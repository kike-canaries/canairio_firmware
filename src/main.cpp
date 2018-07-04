
/**
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018
 * @brief HPMA115S0 sensor on ESP32 with bluetooth GATT notify server
 * @license GPL3
 */

#include <hpma115S0.h>
#include "SSD1306Wire.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

bool DEBUG = true;
// firmware version from git rev-list command
String VERSION_CODE = "rev";
int VCODE = SRC_REV;
//Create an instance of hardware serial
HardwareSerial hpmaSerial(1);
// Display via i2c for WeMOS OLED
SSD1306Wire display(0x3c, 5, 4);
// Create an instance of the hpma115S0 library
#define SAMPLING_RATE 3000
HPMA115S0 hpma115S0(hpmaSerial);
unsigned long count = 0;
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
  display.init();
  display.setLogBuffer(5, 30);
  display.flipScreenVertically();
  display.setContrast(128);
  display.clear();
  Serial.println("-->OLED ready");
}

void showWelcome(){
  display.setLogBuffer(5, 30);     // forcing redraw display (bug)
  display.flipScreenVertically();  // forcing redraw display (bug)
  display.setContrast(128);        // forcing redraw display (bug)
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, display.getHeight()/2, "ESP32 HPMA115");
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(display.getWidth()-5,display.getHeight()-10, VERSION_CODE+VCODE);
  display.display();
  Serial.println("-->Welcome screen ready\n");
  delay(1000);
}

void displayOnBuffer(String msg){
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.println(msg);
  display.drawLogBuffer(0,0);
  display.display();
}

/**
* PM2.5 and PM10 read and visualization functions
*/
void sensorRead(){
  if (hpma115S0.ReadParticleMeasurement(&pm2_5, &pm10)) {
    Serial.print(String(count)+"\tPm2.5:\t" + String(pm2_5) + " ug/m3\t" );
    Serial.println("Pm10:\t" + String(pm10) + " ug/m3" );
    String display = String(count)+" P25: " + String(pm2_5) + " | P10: " + String(pm10);
    if(deviceConnected)displayOnBuffer(display);
    count++;
    delay(SAMPLING_RATE);
  }
}

String sensorGetRead25(){
  return String("{")+"\"P25\":"+String(pm2_5)+"}"; // max supported 20 chars
}

String sensorGetRead10(){
  return String("{")+"\"P10\":"+String(pm10)+"}"; // max supported 20 chars
}

void sensorInit(){
  Serial.print("-->Starting hpma115S0..");
  hpma115S0.Init();
  hpma115S0.StartParticleMeasurement();
  delay(10);
  Serial.println("done");
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
  // Create a BLE Characteristic for PM 10
  pCharactPM10 = pService->createCharacteristic(
                      CHARAC_PM10_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Create a BLE Descriptor
  pCharactPM25->addDescriptor(new BLE2902());
  pCharactPM10->addDescriptor(new BLE2902());
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
    pCharactPM10->setValue(sensorGetRead10().c_str());
    pCharactPM25->notify();
    pCharactPM10->notify();
    delay(100); // bluetooth stack will go into congestion, if too many packets are sent
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("[BLE] start advertising");
    oldDeviceConnected = deviceConnected;
    delay(100);
    showWelcome();
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
  hpmaSerial.begin(9600,SERIAL_8N1,13,15);
  Serial.println("-->HardwareSerial ready");
  delay(10);
  displayInit();
  sensorInit();
  bleServerInit();
  showWelcome();
}

void loop() {
  bleLoop();
}
