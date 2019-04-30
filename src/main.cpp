
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
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <GUIUtils.hpp>
#include <vector>
#include "main.h"
#include "status.h"

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

/******************************************************************************
*   S E N S O R S   M E T H O D S
******************************************************************************/

/***
 * Average methods
 **/

uint32_t getGASAverage(){
  uint32_t avarage_GAS = accumulate( vGAS.begin(), vGAS.end(), 0.0)/vGAS.size();
  vGAS.clear();
  return avarage_GAS;
}

float getAQSAverage(){
  float avarage_AQS = accumulate( vAQS.begin(), vAQS.end(), 0.0)/vAQS.size();
  vAQS.clear();
  return avarage_AQS;
}

float getIAQAverage(){
  float avarage_IAQ = accumulate( vIAQ.begin(), vIAQ.end(), 0.0)/vIAQ.size();
  vIAQ.clear();
  return avarage_IAQ;
}

void averageLoop(){
  if (vIAQ.size() >= cfg.stime){
    aGAS = getGASAverage();  // global var for display
    aAQS = getAQSAverage();
    aIAQ = getIAQAverage();
  }
}

bool isStimeTick(){
  return vIAQ.size()==0;
}

void statusLoop(){
  if (isStimeTick()) {
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
  doc["P25"] = aIAQ;  // notification capacity is reduced, only main value
  String json;
  serializeJson(doc,json);
  return json;
}

String getSensorData(){
  StaticJsonDocument<150> doc;
  doc["aqs"] = aAQS;
  doc["gas"] = aGAS;
  doc["iaq"] = aIAQ;
  doc["tmp"] = tmp;
  doc["hum"] = hum;
  doc["lat"] = cfg.lat;
  doc["lon"] = cfg.lon;
  doc["alt"] = alt;
  doc["spd"] = cfg.spd;
  doc["sta"] = status.to_string().c_str();
  String json;
  serializeJson(doc,json);
  return json;
}

void GetGasReference(){
  // Now run the sensor for a burn-in period, then use combination of relative humidity and gas resistance to estimate indoor air quality as a percentage.
  Serial.println("-->[BME680] New gas reference..");
  int readings = 10;
  for (int i = 1; i <= readings; i++){ // read gas for 10 x 0.150mS = 1.5secs
    gas_reference += bme.readGas();
  }
  gas_reference = gas_reference / readings;
}

String CalculateIAQ(float score){
  score = (100-score)*5;
  String IAQ_text = " IAQ:"+String(score,1)+"=>";
  vIAQ.push_back(score);
  gui.displaySensorAvarage(aIAQ);  // it was calculated on bleLoop()
  if      (score >= 301)                  IAQ_text += "Hazardous";
  else if (score >= 201 && score <= 300 ) IAQ_text += "Very Unhealthy";
  else if (score >= 176 && score <= 200 ) IAQ_text += "Unhealthy";
  else if (score >= 151 && score <= 175 ) IAQ_text += "Unhealthy for Sensitive Groups";
  else if (score >=  51 && score <= 150 ) IAQ_text += "Moderate";
  else if (score >=  00 && score <=  50 ) IAQ_text += "Good";
  return IAQ_text;
}

void bme680loop(){
  Serial.print("-->[BME680] Tmp:");
  tmp = bme.readTemperature();
  Serial.print(tmp);
  Serial.print("°C ");

  Serial.print("Prs:");
  prs = bme.readPressure() / 100.0F;
  Serial.print(prs);
  Serial.print("HPa ");

  Serial.print("Hum:");
  hum = bme.readHumidity();
  Serial.print(hum);
  Serial.print("% ");

  Serial.print("Gas:");
  uint32_t gas = bme.readGas();
  vGAS.push_back(gas);
  Serial.print(gas);
  Serial.print("R ");

  //Calculate humidity contribution to IAQ index
  float current_humidity = bme.readHumidity();
  if (current_humidity >= 38 && current_humidity <= 42)
    hum_score = 0.25*100; // Humidity +/-5% around optimum 
  else
  { //sub-optimal
    if (current_humidity < 38) 
      hum_score = 0.25/hum_reference*current_humidity*100;
    else
    {
      hum_score = ((-0.25/(100-hum_reference)*current_humidity)+0.416666)*100;
    }
  }
  //Calculate gas contribution to IAQ index
  float gas_lower_limit = 5000;   // Bad air quality limit
  float gas_upper_limit = 50000;  // Good air quality limit 
  if (gas_reference > gas_upper_limit) gas_reference = gas_upper_limit; 
  if (gas_reference < gas_lower_limit) gas_reference = gas_lower_limit;
  gas_score = (0.75/(gas_upper_limit-gas_lower_limit)*gas_reference -(gas_lower_limit*(0.75/(gas_upper_limit-gas_lower_limit))))*100;
  //Combine results for the final IAQ index value (0-100% where 100% is good quality air)
  float air_quality_score = hum_score + gas_score;
  vAQS.push_back(air_quality_score);
  //Serial.println("Air Quality = "+String(air_quality_score,1)+"% derived from 25% of Humidity reading and 75% of Gas reading - 100% is good quality air");
  Serial.print("AQS:"+String(air_quality_score,1)+"% ");
  Serial.print("HumE:"+String(hum_score/100)+" ");
  Serial.print("GasE:"+String(gas_score/100)+" ");
  if (gas < 120000) Serial.print("[Poor AQ]");
  Serial.println(CalculateIAQ(air_quality_score));
  if ((getgasreference_count++)%10==0) GetGasReference();
}

void bme680init() {
  bool status = bme.begin(BME680_DEFAULT_ADDRESS);
  if (!status) {
    Serial.println("-->[E][BME680] Could not find a valid BME280 sensor, check wiring!");
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_2X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_2X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320°C for 150 ms
  // Now run the sensor for a burn-in period, then use combination of relative humidity and gas resistance to estimate indoor air quality as a percentage.
  GetGasReference();
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
  if (isStimeTick() && wifiOn && apiIsConfigured()) {
    Serial.print("-->[API] writing to ");
    Serial.print(""+String(api.ip)+"..");
    bool status = api.write(0,0,0,aGAS,aAQS,aIAQ,hum,tmp,prs,cfg.lat,cfg.lon,cfg.alt,cfg.spd,cfg.stime);
    if(status) Serial.println("done");
    else Serial.println("fail! "+String(api.getResponse()));
    dataSendToggle = true;
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
    "iaq=%f,gas=%u,aqs=%f,hum=%f,tmp=%f,lat=%f,lng=%f,alt=%f,spd=%f,stime=%i,tstp=%u",
    aIAQ,aGAS,aAQS,hum,tmp,cfg.lat,cfg.lon,cfg.alt,cfg.spd,cfg.stime,0
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
  if(!influxDbIsConfigured() || aGAS == 0 ) {
    return false;
  }
  char tags[256];
  influxDbAddTags(tags);
  char fields[256];
  influxDbParseFields(fields);
  //canairioWrite(ifxid.c_str(), tags, fields);
  return influx.write(cfg.dname.c_str(), tags, fields);
}

void influxDbLoop() {
  if(isStimeTick() && wifiOn && influxDbIsConfigured()){
    int ifx_retry = 0;
    Serial.print("-->[INFLUXDB] writing to ");
    Serial.print("" + cfg.ifxip + "..");
    while(!influxDbWrite() && ifx_retry++ < IFX_RETRY_CONNECTION){
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
  if(isStimeTick() && cfg.wifiEnable && cfg.ssid.length()>0 && !wifiCheck()) {
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
  if (deviceConnected && isStimeTick()) {  // v25 test for get each ~5 sec aprox
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
  Serial.begin(115200);
  gui.displayInit(u8g2);
  gui.showWelcome();
  cfg.init("canairio");
  Serial.println("\n== INIT SETUP ==\n");
  Serial.println("-->[INFO] ESP32MAC: "+String(cfg.deviceId));
  gui.welcomeAddMessage("BME680 init..");
  bme680init();
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
  averageLoop();   // calculated of sensor data average
  bme680loop();    // read BME680
  bleLoop();       // notify data to connected devices
  wifiLoop();      // check wifi and reconnect it
  apiLoop();
  influxDbLoop();  // influxDB publication
  statusLoop();    // update sensor status GUI
  gui.pageEnd();
  delay(1000);
}
