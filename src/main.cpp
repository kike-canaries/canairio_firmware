
/**
 * @file main.cpp
 * @author Antonio Vanegas @hpsaturn
 * @date June 2018 - 2019
 * @brief HPMA115S0 sensor on ESP32 with bluetooth GATT notify server
 * @license GPL3
 */

#include <Arduino.h>
#include <Wire.h>
#include <InfluxArduino.hpp>
#include <CanAirIoApi.hpp>
#include <ConfigApp.hpp>
#include <ArduinoJson.h>
#include <numeric>
#include <hpma115S0.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>
#include <GUIUtils.hpp>
#include <vector>
#include "main.h"
#include "status.h"

/******************************************************************************
* S E T U P  B O A R D   A N D  F I E L D S
* ---------------------
* please select board on platformio.ini file
******************************************************************************/

#ifdef WEMOSOLED // display via i2c for WeMOS OLED board, pin SCL,SDA
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
#elif HELTEC // display via i2c for Heltec board
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 15, 4, 16);
#elif TTGO18650 // display via i2c for TTGO18650
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
#elif TTGO_TQ // display via i2c for TTGO_TQ
//U8G2_SSD1306_128X32_NONAME_F_SW_I2C u8g2(U8G2_R0, 22, 21, U8X8_PIN_NONE);
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 4, /* data=*/ 5);
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
#elif TTGO18650
#define HPMA_RX 18  // config for TTGO18650 board
#define HPMA_TX 17
#elif TTGO_TQ
#define HPMA_RX 13  // config for TTGO_TQ board
#define HPMA_TX 18
#else
#define HPMA_RX 17  // config for D1MIN1 board
#define HPMA_TX 16
#endif

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
  delay(100);
}

void wrongDataState(){
  Serial.println("-->[E][HPMA] !wrong data!");
  setErrorCode(ecode_sensor_read_fail);
  gui.displaySensorAvarage(apm25);
  gui.displaySensorData(0,0,chargeLevel); 
  hpmaSerial.end();
  statusOff(bit_sensor);
  sensorInit();
  delay(1000);
}

/***
 * Average methods
 **/

void saveDataForAverage(unsigned int pm25, unsigned int pm10){
  v25.push_back(pm25);
  v10.push_back(pm10);
}

unsigned int getPM25Average(){
  unsigned int pm25_average = accumulate( v25.begin(), v25.end(), 0.0)/v25.size();
  v25.clear();
  return pm25_average;
}

unsigned int getPM10Average(){
  unsigned int pm10_average = accumulate( v10.begin(), v10.end(), 0.0)/v10.size();
  v10.clear();
  return pm10_average;
}

void averageLoop(){
  if (v25.size() >= cfg.stime){
    apm25 = getPM25Average();  // global var for display
    apm10 = getPM10Average();   
  }
}

/***
 * PM2.5 and PM10 read and visualization
 **/
void sensorLoop(){
  Serial.print("-->[HPMA] read..");
  int try_sensor_read = 0;
  String txtMsg = "";
  while (txtMsg.length() < 32 && try_sensor_read++ < SENSOR_RETRY) {
    while (hpmaSerial.available() > 0) {
      char inChar = hpmaSerial.read();
      txtMsg += inChar;
      Serial.print(".");
    }
  }
  if(try_sensor_read > SENSOR_RETRY){
    setErrorCode(ecode_sensor_timeout);
    Serial.println("fail"); 
    Serial.println("-->[E][HPMA] disconnected ?"); 
    delay(3000);  // waiting for sensor..
  }
  if (txtMsg[0] == 66) {
    if (txtMsg[1] == 77) {
      Serial.print("done");
      statusOn(bit_sensor);
      unsigned int pm25 = txtMsg[6] * 256 + byte(txtMsg[7]);
      unsigned int pm10 = txtMsg[8] * 256 + byte(txtMsg[9]);
      if(pm25<1000&&pm10<1000){
        gui.displaySensorAvarage(apm25);  // it was calculated on bleLoop()
        gui.displaySensorData(pm25,pm10,chargeLevel); 
        saveDataForAverage(pm25,pm10);
      }
      else wrongDataState();
    }
    else wrongDataState();
  }
  else wrongDataState();
}

void statusLoop(){
  if (v25.size() == 0) {
    Serial.print("-->[STATUS] ");
    Serial.println(status.to_string().c_str());
    updateStatusError();
    wifiCheck();
  }
  gui.updateError(getErrorCode());
  gui.displayStatus(wifiOn,true,deviceConnected,dataSendToggle);
  if(dataSendToggle)dataSendToggle=false;
}

String getNotificationData(){
  StaticJsonDocument<40> doc;
  doc["P25"] = apm25;  // notification capacity is reduced, only main value
  String json;
  serializeJson(doc,json);
  return json;
}

String getSensorData(){
  StaticJsonDocument<150> doc;
  doc["P25"] = apm25;
  doc["P10"] = apm10;
  doc["lat"] = cfg.lat;
  doc["lon"] = cfg.lon;
  doc["alt"] = cfg.alt;
  doc["spd"] = cfg.spd;
  doc["sta"] = status.to_string().c_str();
  String json;
  serializeJson(doc,json);
  return json;
}

void getHumidityRead() {
  humi = am2320.readHumidity();
  temp = am2320.readTemperature();
  if (isnan(humi))
    humi = 0.0;
  if (isnan(temp))
    temp = 0.0;
  Serial.println("-->[AM2320] Humidity: "+String(humi)+" % Temp: "+String(temp)+" °C");
}

void humidityLoop() {
  if (v25.size() == 0) {
    getHumidityRead();
  }
}


/******************************************************************************
*   B A T T E R Y   C H A R G E   S T A T U S   M E T H O D S
******************************************************************************/
// IP5306_2 = pin 27 ESP32, pin 2 IP5306
// IP5306_3 = pin 26 ESP32, pin 3 IP5306

void batteryloop() {

  if (TTGO_TQ == 1) {
      Rdelay = 0;
        digitalWrite(LED, HIGH);
        delayMicroseconds (50);
        digitalWrite(LED, LOW);
      
   while (digitalRead(IP5306_2) == HIGH) {   
        delayMicroseconds (100);             // Sincronization in 1
    }
        delayMicroseconds (50);              // Probably double shoot in 0  
   while (digitalRead(IP5306_2) == HIGH) {   
        delayMicroseconds (100);             // Sincronization in 1
    }
   while (digitalRead(IP5306_2) == LOW && Rdelay < 56) {
        delayMicroseconds (100);             // Sincronization in 0
        Rdelay = Rdelay + 1; 
    }
      if (Rdelay > 52) {
          chargeLevel = 0; // 0%
          Serial.println("Charge level 0%");         
          return;
      }
        delayMicroseconds (1600);
        digitalWrite(LED, HIGH);
        delayMicroseconds (50);
        digitalWrite(LED, LOW);
      if (digitalRead(IP5306_2) == HIGH) {
        delayMicroseconds (100);
        if (digitalRead(IP5306_2) == HIGH) {
          chargeLevel = 4; // 100%
          Serial.println("Charge level 100%");
          return;
        }
    }
      if (digitalRead(IP5306_3) == LOW) {
        delayMicroseconds (100);
        if (digitalRead(IP5306_3) == LOW) {
          chargeLevel = 1; // 25%
          Serial.println("Charge level 25%");
          return;
        }
    }
        delayMicroseconds (1100);
      if (digitalRead(IP5306_3) == HIGH) {
        delayMicroseconds (100);
        if (digitalRead(IP5306_3) == HIGH) {
          chargeLevel = 3; // 75%
          Serial.println("Charge level 75%");
          return;
        }
    }
     if (digitalRead(IP5306_3) == LOW) {
         chargeLevel = 2; // 50%
         Serial.println("Charge level 50%");
         return;
    }
  }        
}

/******************************************************************************
*   C A N A I R I O  P U B L I S H   M E T H O D S
******************************************************************************/

bool apiIsConfigured(){
  return cfg.apiusr.length() > 0 && cfg.apipss.length() > 0 && cfg.dname.length() > 0;
}

void apiInit(){
  if (wifiOn && apiIsConfigured()) {
    Serial.println("-->[API] Connecting..");
    api.configure(cfg.dname.c_str(), cfg.deviceId); // stationId and deviceId, optional endpoint, host and port
    api.authorize(cfg.apiusr.c_str(), cfg.apipss.c_str());
    delay(1000);
  }
}

void apiLoop() {
  if (v25.size() == 0 && wifiOn && apiIsConfigured()) {
    Serial.print("-->[API] writing to ");
    Serial.print(""+String(api.ip)+"..");
    bool status = api.write(0,apm25,apm10,humi,temp,cfg.lat,cfg.lon,cfg.alt,cfg.spd,cfg.stime);
    if(status) {
      Serial.println("done");
      dataSendToggle = true;
    }
    else {
      int code = api.getResponse();
      Serial.println("fail! response code: "+String());
      if (code == -1) {
        Serial.println("-->[E][API] server error rebooting on 5 seg..");
        delay(5000);
      }
    }
  }
}

/******************************************************************************
*   I N F L U X D B   M E T H O D S
******************************************************************************/

bool influxDbIsConfigured(){
  return cfg.ifxdb.length()>0 && cfg.ifxip.length()>0 && cfg.dname.length()>0;
}

void influxDbInit() {
  if(wifiOn && influxDbIsConfigured()) {
    Serial.println("-->[INFLUXDB] connecting..");
    influx.configure(cfg.ifxdb.c_str(), cfg.ifxip.c_str()); //third argument (port number) defaults to 8086
    Serial.print("-->[INFLUXDB] Using HTTPS: ");
    Serial.println(influx.isSecure()); //will be true if you've added the InfluxCert.hpp file.
    cfg.isNewIfxdbConfig=false; // flag for config via BLE
    delay(1000);
  }
}

/**
 * @influxDbParseFields:
 *
 * Supported:
 * "id","pm1","pm25","pm10,"hum","tmp","lat","lng","alt","spd","stime","tstp"
 *
 */
void influxDbParseFields(char* fields){
  sprintf(
    fields,
    "pm1=%u,pm25=%u,pm10=%u,hum=%f,tmp=%f,lat=%f,lng=%f,alt=%f,spd=%f,stime=%i,tstp=%u",
    0,apm25,apm10,humi,temp,cfg.lat,cfg.lon,cfg.alt,cfg.spd,cfg.stime,0
  );
}

void influxDbAddTags(char* tags) {
  // default tag (ESP32 MacAdress)
  if(cfg.ifxtg.length()>0)
    sprintf(tags,"mac=%04X%08X,%s",(uint16_t)(cfg.chipid >> 32),(uint32_t)cfg.chipid,cfg.ifxtg.c_str());
  else
    sprintf(tags,"mac=%04X%08X",(uint16_t)(cfg.chipid >> 32),(uint32_t)cfg.chipid);
}

bool influxDbWrite() {
  if(!influxDbIsConfigured() || apm25 == 0 || apm10 == 0) {
    return false;
  }
  char tags[256];
  influxDbAddTags(tags);
  char fields[256];
  influxDbParseFields(fields);
  return influx.write(cfg.dname.c_str(), tags, fields);
}

void influxDbLoop() {
  if(v25.size()==0 && wifiOn && influxDbIsConfigured()){
    int ifx_retry = 0;
    Serial.print("-->[INFLUXDB] writing to ");
    Serial.print("" + cfg.ifxip + "..");
    while(!influxDbWrite() && ( ifx_retry++ < IFX_RETRY_CONNECTION )){
      Serial.print(".");
      delay(200);
    }
    if(ifx_retry > IFX_RETRY_CONNECTION ) {
      Serial.println("failed!\n-->[E][INFLUXDB] write error, try wifi restart..");
      statusOff(bit_cloud);
      setErrorCode(ecode_ifdb_write_fail);
      wifiRestart();
    }
    else {
      Serial.println("done");
      statusOn(bit_cloud);
      dataSendToggle = true;
    }
  }
}

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
    cfg.isNewWifi=false;  // flag for config via BLE
    Serial.println("done\n-->[WIFI] connected!");
  }
  else{
    Serial.println("fail!\n-->[E][WIFI] disconnected!");
    setErrorCode(ecode_wifi_fail);
  }
}

void wifiInit(){
  if(cfg.wifiEnable && cfg.ssid.length() > 0 && cfg.pass.length() > 0) {
    wifiConnect(cfg.ssid.c_str(), cfg.pass.c_str());
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
  if(v25.size()==0 && cfg.wifiEnable && cfg.ssid.length()>0 && !wifiCheck()) {
    wifiConnect(cfg.ssid.c_str(), cfg.pass.c_str());
    influxDbInit();
    apiInit();
  }
}


/******************************************************************************
*   B L U E T O O T H  M E T H O D S
******************************************************************************/
class MyServerCallbacks: public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
      Serial.println("-->[BLE] onConnect");
      statusOn(bit_paired);
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("-->[BLE] onDisconnect");
      statusOff(bit_paired);
      deviceConnected = false;
    };
}; // BLEServerCallbacks

class MyConfigCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        if(cfg.save(value.c_str())){
          cfg.reload();
          if(cfg.isNewWifi){
            wifiRestart();
            apiInit();
            influxDbInit();
          }
          if(cfg.isNewIfxdbConfig) influxDbInit();
          if(cfg.isNewAPIConfig) apiInit();
          if(!cfg.wifiEnable) wifiStop();
        }
        else{
          setErrorCode(ecode_invalid_config);
        }
        pCharactConfig->setValue(cfg.getCurrentConfig().c_str());
        pCharactData->setValue(getSensorData().c_str());
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
  // Saved current sensor data
  pCharactData->setValue(getSensorData().c_str());
  // Setting Config callback
  pCharactConfig->setCallbacks(new MyConfigCallbacks());
  // Saved current config data
  pCharactConfig->setValue(cfg.getCurrentConfig().c_str());
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
    pCharactData->setValue(getNotificationData().c_str());  // small payload for notification
    pCharactData->notify();
    pCharactData->setValue(getSensorData().c_str());        // load big payload for possible read
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

void setup() {

  pinMode(IP5306_2, INPUT);
  pinMode(IP5306_3, INPUT);

  Serial.begin(115200);
  gui.displayInit(u8g2);
  gui.showWelcome();
  cfg.init("canairio");
  Serial.println("\n== INIT SETUP ==\n");
  Serial.println("-->[INFO] ESP32MAC: "+String(cfg.deviceId));
  gui.welcomeAddMessage("Sensors test..");
  sensorInit();
  am2320.begin();
  bleServerInit();
  gui.welcomeAddMessage("GATT server..");
  if(cfg.ssid.length()>0) gui.welcomeAddMessage("WiFi:"+cfg.ssid);
  else gui.welcomeAddMessage("WiFi radio test..");
  wifiInit();
  gui.welcomeAddMessage("CanAirIO API..");
  influxDbInit();
  apiInit();
  pinMode(LED,OUTPUT);
  gui.welcomeAddMessage("==SETUP READY==");
  delay(500);
}

void loop(){
  gui.pageStart();
  sensorLoop();    // read HPMA serial data and showed it
  averageLoop();   // calculated of sensor data average
  humidityLoop();  // read AM2320
  batteryloop();      // battery charge status 
  bleLoop();       // notify data to connected devices
  wifiLoop();      // check wifi and reconnect it
  apiLoop();
  influxDbLoop();  // influxDB publication
  statusLoop();    // update sensor status GUI
  gui.pageEnd();
  delay(1000);

/* 
  if (resetvar == 599) {
  resetvar = 0;
  ESP.restart();   // 10 minutos
  }
  resetvar = resetvar + 1;
*/
}
