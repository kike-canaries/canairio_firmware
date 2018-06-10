
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

//Create an instance of hardware serial
HardwareSerial hpmaSerial(1);
// Display via i2c for WeMOS OLED
SSD1306Wire display(0x3c, 5, 4);
// Create an instance of the hpma115S0 library
HPMA115S0 hpma115S0(hpmaSerial);
// BLE vars
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t value = 0;

#define SERVICE_UUID        "c8d1d262-861f-4082-947e-f383a259aaf3"
#define CHARACTERISTIC_UUID "b0f332a8-a5aa-4f3f-bb43-f99e7791ae01"

class MyServerCallbacks: public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
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

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());
  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client to notify...");
}

void bleLoop(){
  // notify changed value
    if (deviceConnected) {
        pCharacteristic->setValue(&value, 1);
        pCharacteristic->notify();
        value++;
        delay(10); // bluetooth stack will go into congestion, if too many packets are sent
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}

void displayInit(){
  display.init();
  display.flipScreenVertically();
  display.setContrast(128);
  display.clear();
  Serial.println("-->OLED ready");
}

void showWelcome(){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, display.getHeight()/2, "ESP32 HPMA115S0");
  display.display();
  Serial.println("-->Welcome screen ready");
  delay(1000);
  display.setLogBuffer(5, 30);
}

void displayOnBuffer(String msg){
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.println(msg);
  display.drawLogBuffer(0,0);
  display.display();
}

void sensorInit(){
  Serial.print("Starting hpma115S0..");
  hpma115S0.Init();
  hpma115S0.StartParticleMeasurement();
  Serial.print("done");
}

void sensorLoop(){
  unsigned int pm2_5, pm10;
  if (hpma115S0.ReadParticleMeasurement(&pm2_5, &pm10)) {
    Serial.print("PM 2.5:\t" + String(pm2_5) + " ug/m3\t" );
    Serial.println("\tPM 10:\t" + String(pm10) + " ug/m3" );
    displayOnBuffer("PM25:  " + String(pm2_5) + " | PM10:  " + String(pm10));
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("-->DebugConsole ready");
  hpmaSerial.begin(9600,SERIAL_8N1,13,15);
  Serial.println("-->HardwareSerial ready");
  delay(100);
  displayInit();
  sensorInit();
  bleServerInit();
  showWelcome();
}

void loop() {
  bleLoop();
  sensorLoop();
}
