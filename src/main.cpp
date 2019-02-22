
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
#include <GUIUtils.hpp>
#include <Preferences.h>
#include "WiFi.h"

const char app_name[] = "canairio";
using namespace std;

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
vector<unsigned int> v25;      // for avarage
vector<unsigned int> v10;      // for avarage
unsigned int apm25 = 0;        // last PM2.5 avarage
unsigned int apm10 = 0;        // last PM10 avarage
int stime = 5;                 // sample time (send data each 5 sec)

// WiFi fields
#define WIFI_RETRY_CONNECTION    20
String ssid, pass;
bool dataSendToggle;
bool wifiOn;

// Bluetooth fields
BLEServer* pServer = NULL;
BLECharacteristic* pCharactData = NULL;
BLECharacteristic* pCharactConfig = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
#define SERVICE_UUID        "c8d1d262-861f-4082-947e-f383a259aaf3"
#define CHARAC_DATA_UUID    "b0f332a8-a5aa-4f3f-bb43-f99e7791ae01"
#define CHARAC_CONFIG_UUID  "b0f332a8-a5aa-4f3f-bb43-f99e7791ae02"

// InfluxDB fields
InfluxArduino influx;
String ifxdb, ifxip, ifxuser, ifxpassw, ifxid, ifxfd;

// GUI fields
#define LED 2
GUIUtils gui;

Preferences preferences;

/******************************************************************************
*   S E N S O R  M E T H O D S
******************************************************************************/
/**
 * [DEPRECATED] sensorConfig:
 * The next method is only if sensor was config without autosend
 */

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
  Serial.println("-->[E][HPMA] !wrong data!");
  gui.updateError();
  txtMsg="";
  hpmaSerial.end();
  sensorInit();
  delay(1000);
}

/***
 * Avarage methods
 **/

void saveDataForAvarage(unsigned int pm25, unsigned int pm10){
  v25.push_back(pm25);
  v10.push_back(pm10);
}

unsigned int getPM25Avarage(){
  unsigned int pm25_avarage = accumulate( v25.begin(), v25.end(), 0.0)/v25.size();
  v25.clear();
  return pm25_avarage; 
}

unsigned int getPM10Avarage(){
  unsigned int pm10_avarage = accumulate( v10.begin(), v10.end(), 0.0)/v10.size();
  v10.clear();
  return pm10_avarage; 
}

void avarageLoop(){
  if (v25.size() >= stime){
    apm25 = getPM25Avarage();  // global var for display
    apm10 = getPM10Avarage();
  }
}

/***
 * PM2.5 and PM10 read and visualization
 **/

void sensorLoop(){
  Serial.print("-->[HPMA] read.");
  while (txtMsg.length() < 32) {
    while (hpmaSerial.available() > 0) {
      char inChar = hpmaSerial.read();
      txtMsg += inChar;
      Serial.print(".");
    }
  }
  if (txtMsg[0] == 66) {
    if (txtMsg[1] == 77) {
      Serial.print("done");
      unsigned int pm25 = txtMsg[6] * 256 + byte(txtMsg[7]);
      unsigned int pm10 = txtMsg[8] * 256 + byte(txtMsg[9]);
      txtMsg="";
      if(pm25<1000&&pm10<1000){
        gui.displaySensorAvarage(apm25);  // it was calculated on bleLoop()
        gui.displaySensorData(pm25,pm10);
        saveDataForAvarage(pm25,pm10);
      }
      else wrongDataState();
    }
    else wrongDataState();
  }
  else wrongDataState();
}

void statusLoop(){
  gui.displayStatus(wifiOn,true,deviceConnected,dataSendToggle);
  if(dataSendToggle)dataSendToggle=false;
}

String getFormatData(unsigned int pm25, unsigned int pm10){
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["P25"] = pm25;
  root["P10"] = pm10;
  String json;
  root.printTo(json);
  return json;
}

/******************************************************************************
*   I N F L U X D B   M E T H O D S
******************************************************************************/

void influxDbInit() {
  Serial.println("-->[INFLUXDB] Starting..");
  influx.configure(ifxdb.c_str(), ifxip.c_str()); //third argument (port number) defaults to 8086
  Serial.print("-->[INFLUXDB] Using HTTPS: ");
  Serial.println(influx.isSecure()); //will be true if you've added the InfluxCert.hpp file.
  delay(1000);
}

bool isInfluxDbConfigured(){
  return ifxdb.length()>0 && ifxip.length()>0 && ifxfd.length()>0 && ifxid.length()>0;
}

bool influxDbWrite() {
  if(!isInfluxDbConfigured() || apm25 == 0 || apm10 == 0) {
    return false;
  }
  char tags[16];
  char fields[128];
  sprintf(tags, "read_ok=true");
  // "id","pm1","pm25","pm10,"hum","tmp","lat","lng","alt","spd","stime","tstp"
  uint64_t id=ESP.getEfuseMac();
  sprintf(fields,"id=%X,pm1=%u,pm25=%u,pm10=%u,hum=%d,tmp=%d,lat=%d,lng=%d,alt=%d,spd=%d,stime=%i,tstp=%u",(uint16_t)id,0,apm25,apm10,0,0,0,0,0,0,stime,0);
  return influx.write(ifxid.c_str(), tags, fields);
}

void influxDbReconnect(){
  if (wifiOn && isInfluxDbConfigured()) {
    Serial.println("-->[INFLUXDB] reconnecting..");
    influxDbInit();
  }
}

void influxLoop() {
  if(v25.size()==0 && isInfluxDbConfigured() && wifiOn && influxDbWrite()){
    dataSendToggle=true;
    Serial.println("-->[INFLUXDB] database write ready!");
  }
}

/******************************************************************************
*   W I F I   M E T H O D S
******************************************************************************/

bool wifiCheck(){
  wifiOn = WiFi.isConnected();
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
    Serial.println("done\n-->[WIFI] connected!");
  }
}

void wifiInit(){
  if(ssid.length() > 0 && pass.length() > 0) {
    wifiConnect(ssid.c_str(), pass.c_str());
  }
}

void wifiLoop(){
  if(v25.size()==0 && ssid.length()>0 && !wifiCheck()) {
    wifiConnect(ssid.c_str(), pass.c_str());
    influxDbReconnect();
  }
}

/******************************************************************************
*   C O N F I G  M E T H O D S
******************************************************************************/
String getConfigData(){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  preferences.begin(app_name,false);
  root["ssid"]   =  preferences.getString("ssid",""); // influxdb database name
  root["ifxdb"]  =  preferences.getString("ifxdb",""); // influxdb database name
  root["ifxip"]  =  preferences.getString("ifxip",""); // influxdb database ip
  root["ifxid"]  =  preferences.getString("ifxid",""); // influxdb sensorid name
  root["ifxfd"]  =  preferences.getString("ifxfd",""); // influxdb sensor fields
  root["stime"]  =  preferences.getInt("stime",5);     // sensor measure time
  preferences.end();
  String output;
  root.printTo(output);
  return output;
}

void preferencesInit(){
  preferences.begin(app_name,false);
  ssid = preferences.getString("ssid","");
  pass = preferences.getString("pass","");
  ifxdb = preferences.getString("ifxdb","");
  ifxip = preferences.getString("ifxip","");
  ifxid = preferences.getString("ifxid","");
  ifxfd = preferences.getString("ifxfd","");
  stime = preferences.getInt("stime",5);
  preferences.end();
}

bool saveConfig(const char* json){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("-->[E][CONFIG] parseObject() failed");
    return false;
  }
  String tifxdb = root["ifxdb"] | "";
  String tifxip = root["ifxip"] | "";
  String tifxid = root["ifxid"] | "";
  String tifxfd = root["ifxfd"] | "";
  String tssid  = root["ssid"] | "";
  String tpass  = root["pass"] | "";
  int tstime    = root["stime"] | 5;

  if (tifxdb.length()>0 && tifxip.length()>0 && tifxid.length()>0 && tifxfd.length()>0) {
    preferences.begin(app_name, false);
    preferences.putString("ifxdb", tifxdb );
    preferences.putString("ifxip", tifxip );
    preferences.putString("ifxid", tifxid );
    preferences.putString("ifxfd", tifxfd );
    preferences.end();
    Serial.println("-->[CONFIG] influxdb config saved!");
    Serial.print("-->[CONFIG] ");
    Serial.println(getConfigData());
  }
  else if (tssid.length()>0 && tpass.length()>0){
    preferences.begin(app_name, false);
    preferences.putString("ssid", tssid);
    preferences.putString("pass", tpass);
    preferences.end();
    Serial.println("-->[AUTH] WiFi credentials saved!");
  }
  else if (tstime>0) {
    preferences.begin(app_name, false);
    preferences.putInt("stime", tstime);
    preferences.end();
    Serial.println("-->[CONFIG] sensor sample time saved!");
  }
  else{
    Serial.println("-->[E][CONFIG] invalid config file!");
    return false;
  }
  return true;
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
        if(saveConfig(value.c_str())){
          preferencesInit();
          influxDbReconnect();
          pCharactConfig->setValue(getConfigData().c_str());
        }
        else Serial.println ("-->[E][CONFIG] load config failed!");
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
  // Create a Data Descriptor (for notifications)
  pCharactData->addDescriptor(new BLE2902());
  // Setting Config callback
  pCharactConfig->setCallbacks(new MyConfigCallbacks());
  // Getting saved config data
  pCharactConfig->setValue(getConfigData().c_str());
  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("-->[BLE] GATT server ready. (Waiting for client)");
}

void bleLoop(){
  // notify changed value
  if (deviceConnected && v25.size()==0) {  // v25 test for get each ~5 sec aprox
    Serial.println("-->[BLE] sending notification..");
    pCharactData->setValue(getFormatData(apm25,apm10).c_str());
    pCharactData->notify();
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
*  M A I N
******************************************************************************/

void printDeviceId(){
  uint64_t chipid = ESP.getEfuseMac();                                      //The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("-->[INFO] ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32)); //print High 2 bytes
  Serial.printf("%08X\n", (uint32_t)chipid);                       //print Low 4bytes.
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n== INIT SETUP ==\n");
  printDeviceId();
  Serial.println("-->[SETUP] serial ready.");
  gui.displayInit(u8g2);
  gui.showWelcome();
  sensorInit();
  gui.welcomeAddMessage("Sensor ready..");
  bleServerInit();
  gui.welcomeAddMessage("GATT server..");
  preferencesInit();
  gui.welcomeAddMessage("WiFi test..");
  wifiInit();
  gui.welcomeAddMessage("InfluxDB test..");
  influxDbReconnect();
  pinMode(LED,OUTPUT);
  gui.welcomeAddMessage("==SETUP READY==");
  delay(1000);
}

void loop(){
  gui.pageStart();
  sensorLoop();    // read HPMA serial data and showed it
  avarageLoop();   // calculated of sensor data avarage
  bleLoop();       // notify data to connected devices
  wifiLoop();      // check wifi and reconnect it
  influxLoop();    // influxDB publication
  statusLoop();    // update sensor status GUI
  gui.pageEnd();
  delay(1000);
}
