#include "Sensors.hpp"

HardwareSerial hpmaSerial(1);
HPMA115S0 hpma115S0(hpmaSerial);

bool WrongSerialData = false;
// Sensirium
SPS30 sps30;

// Humidity sensor
Adafruit_AM2320 am2320 = Adafruit_AM2320();

void pmSerialSensorInit() {
    delay(100);
#ifndef TTGO_TQ
    hpmaSerial.begin(9600, SERIAL_8N1, HPMA_RX, HPMA_TX);
#else
    if (WrongSerialData == false) {
        hpmaSerial.begin(9600, SERIAL_8N1, HPMA_RX, HPMA_TX);
    }
    delay(100);
#endif
}

void Sensors::init() {
#ifdef HONEYWELL
    Serial.println("-->[SENSORS] starting hpma115S0 sensor..");
    pmSerialSensorInit();
#elif PANASONIC
    Serial.println("-->[SENSORS] starting SN-GCJA5 sensor..");
    pmSerialSensorInit();
#else  //SENSIRION
    // Begin communication channel
    Serial.println(F("-->[SPS30] starting SPS30 sensor.."));
    if (sps30.begin(SP30_COMMS) == false) {
        Errorloop((char *)"-->[E][SPS30] could not initialize communication channel.", 0);
    }
    // check for SPS30 connection
    if (sps30.probe() == false) {
        Errorloop((char *)"-->[E][SPS30] could not probe / connect with SPS30.", 0);
    } else
        Serial.println(F("-->[SPS30] Detected SPS30."));
    // reset SPS30 connection
    if (sps30.reset() == false) {
        Errorloop((char *)"-->[E][SPS30] could not reset.", 0);
    }
    // start measurement
    if (sps30.start() == true)
        Serial.println(F("-->[SPS30] Measurement OK"));
    else
        Errorloop((char *)"-->[E][SPS30] Could NOT start measurement", 0);
    if (SP30_COMMS == I2C_COMMS) {
        if (sps30.I2C_expect() == 4)
            Serial.println(F("-->[E][SPS30] Due to I2C buffersize only PM values  \n"));
    }
#endif
    // TODO: enable/disable via flag
    am2320.begin();  // temp/humidity sensor
}

// void copyLastVars() {
//     pm1 = tpm1;
//     pm10 = tpm10;
//     pm25 = tpm25;
//     dataReady = true;
//     Serial.print("-->[PMSensor] Saved data: ");
//     Serial.printf("[PM1:%03d][PM2.5:%03d][PM10:%03d]\n", pm1, pm25, pm10);
// }

void _wrongDataState() {
    Serial.println("-->[E][PMSensor] !wrong data!");
}

char _getLoaderChar() {
    char loader[] = {'/', '|', '\\', '-'};
    return loader[random(0, 4)];
}

// void pmsensorEnable(bool enable) {
//     if (enable)
//         digitalWrite(PMS_EN, HIGH);
//     else
//         digitalWrite(PMS_EN, LOW);
//     isPmsensorEnable=enable;
// }

// void pmsensorStart() {
//     pmsensorEnable(true);
// }

// void pmsensorStop() {
//     pmsensorEnable(false);
//     copyLastVars();
//     scount = 0;
// }

// void pmsensorRead() {
//     int try_sensor_read = 0;
//     String txtMsg = "";
//     while (txtMsg.length() < 32 && try_sensor_read++ < SENSOR_RETRY) {
//         while (hpmaSerial.available() > 0) {
//             char inChar = hpmaSerial.read();
//             txtMsg += inChar;
//             Serial.print("-->[PMSensor] read " + String(_getLoaderChar()) + "\r");
//         }
//         Serial.print("-->[PMSensor] read " + String(_getLoaderChar()) + "\r");
//     }
//     if (try_sensor_read > SENSOR_RETRY) {
//         Serial.println("-->[PMSensor] read > fail!");
//         Serial.println("-->[E][PMSensor] disconnected ?");
//         delay(500);  // waiting for sensor..
//     }
//     if (txtMsg[0] == 02) {
//         tpm1 = txtMsg[2] * 256 + byte(txtMsg[1]);
//         tpm25 = txtMsg[6] * 256 + byte(txtMsg[5]);
//         tpm10 = txtMsg[10] * 256 + byte(txtMsg[9]);
//         Serial.print("-->[PMSensor]");
//         Serial.printf("[S%02d][PM1:%03d][PM2.5:%03d][PM10:%03d]\n", ++scount, tpm1, tpm25, tpm10);
//     } else
//         _wrongDataState();
// }

// void pmsensorLoop(bool isBleConnected) {
//     static uint_fast64_t pmLoopTimeStamp = 0;            // timestamp for loop check
//     if ((millis() - pmLoopTimeStamp > 1000)) {
//         pmLoopTimeStamp = millis();
//         static uint64_t pmTimeStamp = 0;            // timestamp for interval and sensor sample time
//         int turnon_interval = SENSOR_INTERVAL;
//         if (isBleConnected) turnon_interval = turnon_interval / 3;
//         if ((millis() - pmTimeStamp > turnon_interval)) {
//             pmsensorStart();
//             if ((millis() - pmTimeStamp) > (turnon_interval + SENSOR_SAMPLE)) {
//                 pmsensorRead();
//                 pmTimeStamp = millis();
//                 pmsensorStop();
//                 Serial.println("-->[PMSensor] Disabled.");
//             } else if ((millis() - pmTimeStamp > turnon_interval + SENSOR_SAMPLE / 2)) {
//                 pmsensorRead();
//             } else {
//                 Serial.println("-->[PMSensor] Waiting for stable measure..");
//             }
//         } else if (isInitSetup && (millis() - pmTimeStamp > 5000)) {
//             pmsensorStart();
//             pmsensorRead();
//             pmTimeStamp = millis();
//             if (initSetupCount++ > 4) {
//                 isInitSetup = false;
//                 pmsensorStop();
//                 Serial.println("-->[PMSensor] Setup done.");
//             }
//         }
//     }
// }

void Sensors::getHumidityRead() {
    humi = am2320.readHumidity();
    temp = am2320.readTemperature();
    if (isnan(humi))
        humi = 0.0;
    if (isnan(temp))
        temp = 0.0;
    Serial.println("-->[AM2320] Humidity: " + String(humi) + " % Temp: " + String(temp) + " °C");
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

////////////////////////////////

/******************************************************************************
*   S E N S O R  M E T H O D S
******************************************************************************/

/**
 * [DEPRECATED] sensorConfig:
 * The next method is only if Honeywell sensor was config without autosend
 */

#ifdef HONEYWELL
void sensorConfig() {
    Serial.println("-->[HPMA] configuration hpma115S0 sensor..");
    hpmaSerial.begin(9600, SERIAL_8N1, HPMA_RX, HPMA_TX);
    hpma115S0.Init();
    delay(100);
    hpma115S0.EnableAutoSend();
    delay(100);
    hpma115S0.StartParticleMeasurement();
    delay(100);
    Serial.println("-->[HPMA] sensor configured.");
}
#endif

#ifdef SENSIRION
void ErrtoMess(char *mess, uint8_t r) {
    char buf[80];
    Serial.print(mess);
    sps30.GetErrDescription(r, buf, 80);
    Serial.println(buf);
}

void Errorloop(char *mess, uint8_t r) {
    if (r)
        ErrtoMess(mess, r);
    else
        Serial.println(mess);
    setErrorCode(ecode_sensor_timeout);
    delay(500);  // waiting for sensor..
}
#endif


/***
 * Average methods
 **/

// void saveDataForAverage(unsigned int pm25, unsigned int pm10){
//   v25.push_back(pm25);
//   v10.push_back(pm10);
// }

// unsigned int getPM25Average(){
//   unsigned int pm25_average = round(accumulate(v25.begin(), v25.end(), 0.0) / v25.size());
//   v25.clear();
//   return pm25_average;
// }

// unsigned int getPM10Average(){
//   unsigned int pm10_average = round(accumulate(v10.begin(), v10.end(), 0.0) / v10.size());
//   v10.clear();
//   return pm10_average;
// }

char getLoaderChar() {
    char loader[] = {'/', '|', '\\', '-'};
    return loader[random(0, 4)];
}

/***
 * PM2.5 and PM10 read and visualization
 **/

void wrongDataState() {
#ifdef HONEYWELL
    Serial.print("-->[E][HPMA] !wrong data!");
#ifndef TTGO_TQ
    hpmaSerial.end();
#endif
#elif PANASONIC
    Serial.print("-->[E][SNGC] !wrong data!");
#ifndef TTGO_TQ
    hpmaSerial.end();
#endif
#else
    Serial.print("-->[E][SPS30] !wrong data!");
#endif
    WrongSerialData = true;
    init();
    delay(500);
}

void Sensors::loop() {
#ifndef SENSIRION
    int try_sensor_read = 0;
    String txtMsg = "";
    while (txtMsg.length() < 32 && try_sensor_read++ < SENSOR_RETRY) {
        while (hpmaSerial.available() > 0) {
            char inChar = hpmaSerial.read();
            txtMsg += inChar;
#ifdef HONEYWELL
            Serial.print("-->[HPMA] read " + String(getLoaderChar()) + "\r");
#else
            Serial.print("-->[SNGC] read " + String(getLoaderChar()) + "\r");
#endif
        }
#ifdef HONEYWELL
        Serial.print("-->[HPMA] read " + String(getLoaderChar()) + "\r");
#else
        Serial.print("-->[SNGC] read " + String(getLoaderChar()) + "\r");
#endif
    }
    if (try_sensor_read > SENSOR_RETRY) {
#ifdef HONEYWELL
        Serial.println("-->[HPMA] read > fail!");
        Serial.println("-->[E][HPMA] disconnected ?");
#else
        Serial.println("-->[SNGC] read > fail!");
        Serial.println("-->[E][SNGC] disconnected ?");
#endif
        delay(500);  // waiting for sensor..
    }
#endif

#ifdef PANASONIC
    if (txtMsg[0] == 02) {
        Serial.print("-->[SNGC] read > done!");
        pm25 = txtMsg[6] * 256 + byte(txtMsg[5]);
        pm10 = txtMsg[10] * 256 + byte(txtMsg[9]);
        if (pm25 > 2000 && pm10 > 2000) {
            wrongDataState();
        }
    } else
        wrongDataState();

#elif HONEYWELL  // HONEYWELL
    if (txtMsg[0] == 66) {
        if (txtMsg[1] == 77) {
            Serial.print("-->[HPMA] read > done!");
            statusOn(bit_sensor);
            pm25 = txtMsg[6] * 256 + byte(txtMsg[7]);
            pm10 = txtMsg[8] * 256 + byte(txtMsg[9]);
            if (pm25 > 1000 && pm10 > 1000) {
                wrongDataState();
            }
        } else
            wrongDataState();
    } else
        wrongDataState();

#else  // SENSIRION
    delay(35);  //Delay for sincronization
    do {
        ret = sps30.GetValues(&val);
        if (ret == ERR_DATALENGTH) {
            if (error_cnt++ > 3) {
                ErrtoMess((char *)"-->[E][SPS30] Error during reading values: ", ret);
                return;
            }
            delay(1000);
        } else if (ret != ERR_OK) {
            ErrtoMess((char *)"-->[E][SPS30] Error during reading values: ", ret);
            return;
        }
    } while (ret != ERR_OK);

    Serial.print("-->[SPS30] read > done!");
    statusOn(bit_sensor);

    pm25 = round(val.MassPM2);
    pm10 = round(val.MassPM10);

    if (pm25 > 1000 && pm10 > 1000) {
        wrongDataState();
    }
#endif
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SENSORSHANDLER)
Sensors sensors;
#endif
