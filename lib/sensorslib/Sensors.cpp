#include "Sensors.hpp"

// Honeywell and Panasonic
#ifndef SENSIRION
HardwareSerial hpmaSerial(1);
HPMA115S0 hpma115S0(hpmaSerial);
#endif

// Sensirium
#ifdef SENSIRION
SPS30 sps30;
#endif
// Humidity sensor
Adafruit_AM2320 am2320 = Adafruit_AM2320();


/******************************************************************************
*   S E N S O R  M E T H O D S
******************************************************************************/

void Sensors::pmsensorRead(){
#ifndef SENSIRION
    int try_sensor_read = 0;
    String txtMsg = "";
    while (txtMsg.length() < 32 && try_sensor_read++ < SENSOR_RETRY) {
        while (hpmaSerial.available() > 0) {
            char inChar = hpmaSerial.read();
            txtMsg += inChar;
        }
    }
    if (try_sensor_read > SENSOR_RETRY) {
        onPmSensorErr("pm sensor read fail!");
        delay(500);  // waiting for sensor..
    }
#endif

#ifdef PANASONIC
    if (txtMsg[0] == 02) {
        Serial.print("-->[SNGC] read > done!");
        pm25 = txtMsg[6] * 256 + byte(txtMsg[5]);
        pm10 = txtMsg[10] * 256 + byte(txtMsg[9]);
        if (pm25 > 2000 && pm10 > 2000) {
            onPmSensorErr("Panasonic out of range pm25 > 2000");
        }
    } else
        onPmSensorErr("invalid Panasonic sensor header!");

#elif HONEYWELL  // HONEYWELL
    if (txtMsg[0] == 66) {
        if (txtMsg[1] == 77) {
            Serial.print("-->[HPMA] read > done!");
            pm25 = txtMsg[6] * 256 + byte(txtMsg[7]);
            pm10 = txtMsg[8] * 256 + byte(txtMsg[9]);
            if (pm25 > 1000 && pm10 > 1000) {
              onPmSensorErr("Honeywell out of range pm25 > 1000");
            }
        } else
            onPmSensorErr("invalid Panasonic sensor header!");
    } else
        onPmSensorErr("invalid Panasonic sensor header!");
#elif SENSIRION
    delay(35);  //Delay for sincronization
    do {
        ret = sps30.GetValues(&val);
        if (ret == ERR_DATALENGTH) {
            if (error_cnt++ > 3) {
                pmSensirionErrtoMess((char *)"-->[E][SPS30] Error during reading values: ", ret);
                return;
            }
            delay(1000);
        } else if (ret != ERR_OK) {
            pmSensirionErrtoMess((char *)"-->[E][SPS30] Error during reading values: ", ret);
            return;
        }
    } while (ret != ERR_OK);

    Serial.print("-->[SPS30] read > done!");

    pm25 = round(val.MassPM2);
    pm10 = round(val.MassPM10);

    if (pm25 > 1000 && pm10 > 1000) {
        onPmSensorErr("Sensirion out of range pm25 > 1000");
    }
#endif
}

void Sensors::am2320Read() {
    humi = am2320.readHumidity();
    temp = am2320.readTemperature();
    if (isnan(humi))
        humi = 0.0;
    if (isnan(temp))
        temp = 0.0;
    Serial.println("-->[AM2320] Humidity: " + String(humi) + " % Temp: " + String(temp) + " °C");
}

void Sensors::setErrorCallBack(voidCbFn cb){
    _onErrorCb = cb;
}

void Sensors::loop() {
    am2320Read();
    pmsensorRead();
}

void Sensors::pmSensorInit() {
#if defined HONEYWELL || defined PANASONIC
    hpmaSerial.begin(9600, SERIAL_8N1, HPMA_RX, HPMA_TX);
    delay(100);
#endif
}



void Sensors::onPmSensorErr(const char *msg) {
    Serial.print("-->[E][PMSENSOR] ");
    Serial.println(msg);
    if(_onErrorCb)_onErrorCb(msg);
}

void Sensors::pmSensirionErrtoMess(char *mess, uint8_t r) {
#ifdef SENSIRION
    char buf[80];
    Serial.print(mess);
    sps30.GetErrDescription(r, buf, 80);
    Serial.println(buf);
    onPmSensorErr(mess);
#endif
}

void Sensors::pmSensirionErrorloop(char *mess, uint8_t r) {
#ifdef SENSIRION
    if (r)
        pmSensirionErrtoMess(mess, r);
    else
        Serial.println(mess);
    delay(500);  // waiting for sensor..
#endif
}

void Sensors::pmSensirionInit() {
#ifdef SENSIRION
    // Begin communication channel
    Serial.println(F("-->[SPS30] starting SPS30 sensor.."));
    if (sps30.begin(SP30_COMMS) == false) {
        pmSensirionErrorloop((char *)"-->[E][SPS30] could not initialize communication channel.", 0);
    }
    // check for SPS30 connection
    if (sps30.probe() == false) {
        pmSensirionErrorloop((char *)"-->[E][SPS30] could not probe / connect with SPS30.", 0);
    } else
        Serial.println(F("-->[SPS30] Detected SPS30."));
    // reset SPS30 connection
    if (sps30.reset() == false) {
        pmSensirionErrorloop((char *)"-->[E][SPS30] could not reset.", 0);
    }
    // start measurement
    if (sps30.start() == true)
        Serial.println(F("-->[SPS30] Measurement OK"));
    else
        pmSensirionErrorloop((char *)"-->[E][SPS30] Could NOT start measurement", 0);
    if (SP30_COMMS == I2C_COMMS) {
        if (sps30.I2C_expect() == 4)
            Serial.println(F("-->[E][SPS30] Due to I2C buffersize only PM values  \n"));
    }
#endif
}

void Sensors::am2320Init() {
    // pinMode(21, INPUT_PULLUP);  ???
    // pinMode(22, INPUT_PULLUP);  ???
    am2320.begin();  // temp/humidity sensor
}

/**
 * All sensors init()
 * 
 * Particle meter and AM2320 sensors init.
 * Please see the platformio.ini file for 
 * know what sensors is enable
 */

void Sensors::init() {
    Serial.println("-->[SENSORS] starting PM sensor..");
#if defined HONEYWELL || defined PANASONIC
    pmSensorInit();
#else  //SENSIRION
    pmSensirionInit();
#endif
    // TODO: enable/disable via flag
    Serial.println("-->[SENSORS] starting AM2320 sensor..");
    am2320Init();
}

void Sensors::restart(){
#if defined HONEYWELL || defined PANASONIC
    hpmaSerial.end();
#endif
    init();
    delay(500);
}

bool Sensors::isDataReady() {
    return dataReady;
}

uint16_t Sensors::getPM1() {
    return pm1;
}

String Sensors::getStringPM1() {
    char output[5];
    sprintf(output, "%03d", getPM1());
    return String(output);
}

uint16_t Sensors::getPM25() {
    return pm25;
}

String Sensors::getStringPM25() {
    char output[5];
    sprintf(output, "%03d", getPM25());
    return String(output);
}

uint16_t Sensors::getPM10() {
    return pm10;
}

String Sensors::getStringPM10() {
    char output[5];
    sprintf(output, "%03d", getPM10());
    return String(output);
}

float Sensors::getHumidity() {
    return humi;
}

float Sensors::getTemperature() {
    return temp;
}

float Sensors::getGas() {
    return gas;
}

float Sensors::getAltitude() {
    return alt;
}

float Sensors::getPressure() {
    return pres;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SENSORSHANDLER)
Sensors sensors;
#endif