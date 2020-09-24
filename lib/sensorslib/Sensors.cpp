#include "Sensors.hpp"

// Serial PM sensor (Honeywell and Panasonic)
#ifndef SENSIRION
HardwareSerial pmsSerial(1);
#endif

// Sensirium
#ifdef SENSIRION
SPS30 sps30;
#endif
// Humidity sensor
Adafruit_AM2320 am2320 = Adafruit_AM2320();


/******************************************************************************
*   S E N S O R   P R I V A T E   M E T H O D S
******************************************************************************/

bool Sensors::pmsensorRead(){
#ifndef SENSIRION
    int try_sensor_read = 0;
    String txtMsg = "";
    while (txtMsg.length() < 32 && try_sensor_read++ < SENSOR_RETRY) {
        while (pmsSerial.available() > 0) {
            char inChar = pmsSerial.read();
            txtMsg += inChar;
        }
    }
    if (try_sensor_read > SENSOR_RETRY) {
        onPmSensorError("pm sensor read fail!");
        return false;
    }
#endif

#ifdef PANASONIC
    if (txtMsg[0] == 02) {
        if(debug) Serial.print("-->[SNGC] read > done!");
        pm25 = txtMsg[6] * 256 + byte(txtMsg[5]);
        pm10 = txtMsg[10] * 256 + byte(txtMsg[9]);
        if (pm25 > 2000 && pm10 > 2000) {
            onPmSensorError("Panasonic out of range pm25 > 2000");
            return false;
        }
    } else {
        onPmSensorError("invalid Panasonic sensor header!");
        return false;
    }

#elif HONEYWELL  // HONEYWELL
    if (txtMsg[0] == 66) {
        if (txtMsg[1] == 77) {
            if (debug) Serial.print("-->[HPMA] read > done!");
            pm25 = txtMsg[6] * 256 + byte(txtMsg[7]);
            pm10 = txtMsg[8] * 256 + byte(txtMsg[9]);
            if (pm25 > 1000 && pm10 > 1000) {
                onPmSensorError("Honeywell out of range pm25 > 1000");
                return false;
            }
        } else {
            onPmSensorError("invalid Panasonic sensor header!");
            return false;
        }
    } else {
        onPmSensorError("invalid Panasonic sensor header!");
        return false;
    }

#elif SENSIRION
    delay(35);  //Delay for sincronization
    do {
        ret = sps30.GetValues(&val);
        if (ret == ERR_DATALENGTH) {
            if (error_cnt++ > 3) {
                pmSensirionErrtoMess((char *)"-->[W][SPS30] Error during reading values: ", ret);
                return false;
            }
            delay(1000);
        } else if (ret != ERR_OK) {
            pmSensirionErrtoMess((char *)"-->[W][SPS30] Error during reading values: ", ret);
            return false;
        }
    } while (ret != ERR_OK);

    if (debug) Serial.print("-->[SPS30] read > done!");

    pm25 = round(val.MassPM2);
    pm10 = round(val.MassPM10);

    if (pm25 > 1000 && pm10 > 1000) {
        onPmSensorError("Sensirion out of range pm25 > 1000");
        return false;
    }
#endif
    return true;
}

void Sensors::am2320Read() {
    humi = am2320.readHumidity();
    temp = am2320.readTemperature();
    if (isnan(humi)) humi = 0.0;
    if (isnan(temp)) temp = 0.0;
}

void Sensors::onPmSensorError(const char *msg) {
    Serial.print("-->[W][PMSENSOR] ");
    Serial.println(msg);
    if(_onErrorCb)_onErrorCb(msg);
}

void Sensors::pmSensirionErrtoMess(char *mess, uint8_t r) {
#ifdef SENSIRION
    char buf[80];
    Serial.print("-->[W][SENSIRION] ");
    Serial.print(mess);
    sps30.GetErrDescription(r, buf, 80);
    Serial.println(buf);
    onPmSensorError(mess);
#endif
}

void Sensors::pmSensirionErrorloop(char *mess, uint8_t r) {
#ifdef SENSIRION
    if (r) pmSensirionErrtoMess(mess, r);
    else Serial.println(mess);
#endif
}
/**
 * Particule meter sensor init.
 * 
 * Hardware serial init for multiple sensors, like
 * Honeywell, Plantower, Panasonic.
 * 
 * @param PMS_RX defined for RX wire.
 * @param PMS_TX defined for TX wire.
 **/
void Sensors::pmSensorInit() {
#if defined HONEYWELL || defined PANASONIC
    Serial.println(F("-->[PMSENSOR] starting HPMA/PANASONIC sensor.."));
    pmsSerial.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);
    delay(100);
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
    am2320.begin();  // temp/humidity sensor
}

/// Print some sensors values
void Sensors::printValues() {
    if (debug) {
        char output[100];
        sprintf(output, " PM1:%03d PM25:%03d PM10:%03d H:%02d%% T:%02d°C", pm1, pm25, pm10, (int)humi, (int)temp);
        Serial.println(output);
    }
}

/***********************************************************************************
 *  P U B L I C   M E T H O D S
 * *********************************************************************************/

/**
 * Main sensor loop.
 * All sensor read methods here, please call it on main loop.
 */
void Sensors::loop() {
    static uint_fast64_t pmLoopTimeStamp = 0;                 // timestamp for sensor loop check data
    if ((millis() - pmLoopTimeStamp > sample_time * 1000)) {  // sample time for each capture
        dataReady = false;
        pmLoopTimeStamp = millis();
        am2320Read();
        bool pmsuccess = pmsensorRead();
        printValues();
        if(_onDataCb && pmsuccess) _onDataCb();
        dataReady = true;
    }
}

/**
 * All sensors init.
 * 
 * Particle meter and AM2320 sensors init.
 * Please see the platformio.ini file for 
 * know what sensors is enable.
 * 
 * @param debug enable PM sensor log output.
 */
void Sensors::init(bool debug) {
    debug = debug;
    if (!debug) Serial.println("-->[SENSORS] debugging is disable.");

    Serial.print("-->[SENSORS] sample time set to: ");
    Serial.println(sample_time);

    Serial.println("-->[SENSORS] starting PM sensor..");
#if defined HONEYWELL || defined PANASONIC
    pmSensorInit();
#else  //SENSIRION
    pmSensirionInit();
#endif

    // TODO: enable/disable via flag
    Serial.println("-->[AM2320] starting AM2320 sensor..");
    am2320Init();
}

/// set loop time interval for each sensor sample
void Sensors::setSampleTime(int seconds){
    sample_time = seconds;
}

void Sensors::restart(){
#if defined HONEYWELL || defined PANASONIC
    pmsSerial.end();
#endif
    init();
    delay(100);
}

void Sensors::setOnDataCallBack(voidCbFn cb){
    _onDataCb = cb;
}

void Sensors::setOnErrorCallBack(errorCbFn cb){
    _onErrorCb = cb;
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
