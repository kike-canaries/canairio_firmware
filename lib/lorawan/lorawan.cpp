#include "lorawan.h"
#include <hal/hal.h>
#include <SPI.h>
#include "io_pins.h"
#include "functions.h"

#include "Batterylib.hpp"
#include <CayenneLPP.h>
#include <Sensors.hpp>

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = PIN_LMIC_NSS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = PIN_LMIC_RST,
    .dio = {PIN_LMIC_DIO0, PIN_LMIC_DIO1, PIN_LMIC_DIO2},
};


// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = LORA_TX_INTERVAL;

void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }

bool GO_DEEP_SLEEP = false;

RTC_DATA_ATTR lmic_t RTC_LMIC;

JsonDocument jsonBuffer;
CayenneLPP lpp(160);
//JsonObject root = jsonBuffer.to<JsonObject>();
JsonObject LORA_DATA = jsonBuffer.to<JsonObject>();

void LoRaWANSetup()
{
   

    Serial.println(F("LoRaWAN_Setup ..."));

    Serial.print(F("Saved seqnoUp: "));
    Serial.println(LMIC.seqnoUp);

    // LMIC init
    os_init();

    // Let LMIC compensate for +/- 1% clock error
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    if (RTC_LMIC.seqnoUp != 0)
    {
        LoraWANLoadLMICFromRTC();
    }

    // Start job
    LoraWANDo_send(&sendjob);
}

void LoraWANDo_send(osjob_t *j)
{
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else if (LMIC.opmode & OP_TXDATA) {
        Serial.println(F("OP_TXDATA, not sending"));
    } else {
        LoraWANGetData();

        // Prepare upstream data transmission at the next possible time.  
        //LMIC_setTxData2(1, LORA_DATA, sizeof(LORA_DATA)-1, 0);
        LMIC_setTxData2(1, (lpp.getBuffer()), sizeof(lpp.getSize()), 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent(ev_t ev)
{
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev)
    {
    case EV_SCAN_TIMEOUT:
        Serial.println(F("EV_SCAN_TIMEOUT"));
        break;
    case EV_BEACON_FOUND:
        Serial.println(F("EV_BEACON_FOUND"));
        break;
    case EV_BEACON_MISSED:
        Serial.println(F("EV_BEACON_MISSED"));
        break;
    case EV_BEACON_TRACKED:
        Serial.println(F("EV_BEACON_TRACKED"));
        break;
    case EV_JOINING:
        Serial.println(F("EV_JOINING"));
        break;
    case EV_JOINED:
        Serial.println(F("EV_JOINED"));
#ifndef DISABLE_JOIN
        {
            u4_t netid = 0;
            devaddr_t devaddr = 0;
            u1_t nwkKey[16];
            u1_t artKey[16];
            LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
            Serial.print("netid: ");
            Serial.println(netid, DEC);
            Serial.print("devaddr: ");
            Serial.println(devaddr, HEX);
            Serial.print("artKey: ");
            for (size_t i = 0; i < sizeof(artKey); ++i)
            {
                Serial.print(artKey[i], HEX);
            }
            Serial.println("");
            Serial.print("nwkKey: ");
            for (size_t i = 0; i < sizeof(nwkKey); ++i)
            {
                Serial.print(nwkKey[i], HEX);
            }
            Serial.println("");
        }
        // Disable link check validation (automatically enabled
        // during join, but because slow data rates change max TX
        // size, we don't use it in this example.
        LMIC_setLinkCheckMode(0);
#endif
        break;
    /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
    case EV_JOIN_FAILED:
        Serial.println(F("EV_JOIN_FAILED"));
        break;
    case EV_REJOIN_FAILED:
        Serial.println(F("EV_REJOIN_FAILED"));
        break;
    case EV_TXCOMPLETE:
        Serial.println(F("EV_TXCOMPLETE"));

        if (LMIC.txrxFlags & TXRX_ACK)
        {
            Serial.println(F("Received ack"));
        }

        if (LMIC.dataLen)
        {
            Serial.print(LMIC.dataLen);
            Serial.println(F(" bytes of payload"));
        }

        GO_DEEP_SLEEP = false; //true

        break;
    case EV_LOST_TSYNC:
        Serial.println(F("EV_LOST_TSYNC"));
        break;
    case EV_RESET:
        Serial.println(F("EV_RESET"));
        break;
    case EV_RXCOMPLETE:
        // data received in ping slot
        Serial.println(F("EV_RXCOMPLETE"));
        break;
    case EV_LINK_DEAD:
        Serial.println(F("EV_LINK_DEAD"));
        break;
    case EV_LINK_ALIVE:
        Serial.println(F("EV_LINK_ALIVE"));
        break;
    /* This event is defined but not used in the code.
        case EV_SCAN_FOUND:
            DisplayPrintln(F("EV_SCAN_FOUND"), LORAWAN_STATE_DISPLAY_LINE);
            break;
        */
    case EV_TXSTART:
        Serial.println(F("EV_TXSTART"));
        break;
    case EV_TXCANCELED:
        Serial.println(F("EV_TXCANCELED"));
        break;
    case EV_RXSTART:
        /* do not print anything -- it wrecks timing */
        break;
    case EV_JOIN_TXCOMPLETE:
        Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
        break;
    default:
        Serial.print(F("Unknown event: "));
        Serial.println((unsigned)ev);
        break;
    }
}


void LoraWANDo(void)
{
    long seconds = millis() / 1000;
    static unsigned long last_runntime_info = 0;
    static unsigned long runntime_info_ever_ms = 5000;

    if (GO_DEEP_SLEEP == true && !os_queryTimeCriticalJobs(ms2osticksRound((LORA_TX_INTERVAL * 1000))))
    {
        Serial.println(F("Go to DeepSleep ..."));
        Serial.print(F("Runtime was: "));
        Serial.print(seconds);
        Serial.println(F(" seconds"));

        LoraWANSaveLMICToRTC(LORA_TX_INTERVAL);
        Serial.flush();

        //PowerDeepSleepTimer(LORA_TX_INTERVAL - 30 - 8); // 30sec for SDS011, 8 sec for remaining code 
    }
    else
    {
        if(last_runntime_info + runntime_info_ever_ms <  millis()) 
        { 
            Serial.print("Runtime: ");
            Serial.print(seconds);
            Serial.println(" seconds");
            #ifndef PRINTDEBUGS
                LoraWANDebug(LMIC);
            #endif
            last_runntime_info = millis();
        }

        os_runloop_once();
    }
}

void LoraWANGetData()
{ 
    float humi = sensors.getHumidity();
    if (humi == 0.0) humi = sensors.getCO2humi();
    float temp = sensors.getTemperature();
    if (temp == 0.0) temp = sensors.getCO2temp();
    lpp.reset();
    lpp.addGenericSensor(1,(sensors.getPM1()));
    lpp.addGenericSensor(2,(sensors.getPM25()));
    lpp.addGenericSensor(3,sensors.getPM10());
    lpp.addGenericSensor(4,sensors.getCO2());
    lpp.addGenericSensor(5,sensors.getCO2humi());
    lpp.addGenericSensor(6,sensors.getCO2temp());
    lpp.addTemperature(7,temp);
    lpp.addRelativeHumidity(8,humi);
    //lpp.addGPS(9,geo);
    lpp.addBarometricPressure(10,(sensors.getPressure()));
    lpp.addGenericSensor(11,(sensors.getGas()));
    lpp.addGenericSensor(12,(sensors.getNH3()));
    lpp.addGenericSensor(13,(sensors.getCO()));
    lpp.addAltitude(14,(sensors.getAltitude()));
    lpp.addGenericSensor(15,(battery.getCharge()));
    lpp.addVoltage(16,(battery.getVoltage()));
    //lpp.addGenericSensor(17,(WiFi.RSSI()));
    lpp.addGenericSensor(18,(ESP.getFreeHeap()));
    //lpp.addGenericSensor(19,(cfg.getStationName().c_str()));
    //lpp.addGenericSensor(20,(cfg.getVersion()));
    //lpp.addGenericSensor(21,(cfg.deviceId.c_str()));
    
    lpp.decodeTTN(lpp.getBuffer(), lpp.getSize(), LORA_DATA);
    serializeJsonPretty(LORA_DATA, Serial);
    
    Serial.println();
}

void LoraWANSaveLMICToRTC(int deepsleep_sec)
{
    Serial.println(F("Save LMIC to RTC ..."));
    RTC_LMIC = LMIC;

    //System time is resetted after sleep. So we need to calculate the dutycycle with a resetted system time
    unsigned long now = millis();

    // EU Like Bands
#if defined(CFG_LMIC_EU_like)
    for(int i = 0; i < MAX_BANDS; i++) {
        ostime_t correctedAvail = RTC_LMIC.bands[i].avail - ((now/1000.0 + deepsleep_sec ) * OSTICKS_PER_SEC);
        if(correctedAvail < 0) {
            correctedAvail = 0;
        }
        RTC_LMIC.bands[i].avail = correctedAvail;
    }

    RTC_LMIC.globalDutyAvail = RTC_LMIC.globalDutyAvail - ((now/1000.0 + deepsleep_sec ) * OSTICKS_PER_SEC);
    if(RTC_LMIC.globalDutyAvail < 0) 
    {
        RTC_LMIC.globalDutyAvail = 0;
    }
#else
    Serial.println("No DutyCycle recalculation function!")
#endif

    #ifndef PRINTDEBUGS
        LoraWANDebug(RTC_LMIC);
    #endif
}

void LoraWANLoadLMICFromRTC()
{
    Serial.println(F("Load LMIC vars from RTC ..."));
    LMIC = RTC_LMIC;

    #ifndef PRINTDEBUGS
        LoraWANDebug(RTC_LMIC);
    #endif
}

void LoraWANPrintVersion(void)
{
    Serial.print(F("LMIC Version: "));
    Serial.print(ARDUINO_LMIC_VERSION_GET_MAJOR (ARDUINO_LMIC_VERSION) );
    Serial.print(F("."));
    Serial.print(ARDUINO_LMIC_VERSION_GET_MINOR (ARDUINO_LMIC_VERSION) );
    Serial.print(F("."));
    Serial.print(ARDUINO_LMIC_VERSION_GET_PATCH (ARDUINO_LMIC_VERSION) );
    Serial.print(F("."));
    Serial.println(ARDUINO_LMIC_VERSION_GET_LOCAL (ARDUINO_LMIC_VERSION) );  
}

// opmode def 
// https://github.com/mcci-catena/arduino-lmic/blob/89c28c5888338f8fc851851bb64968f2a493462f/src/lmic/lmic.h#L233
void LoraWANPrintLMICOpmode(void)
{
    Serial.print(F("LMIC.opmode: "));
    if (LMIC.opmode & OP_NONE) { Serial.print(F("OP_NONE ")); }
    if (LMIC.opmode & OP_SCAN) { Serial.print(F("OP_SCAN ")); }
    if (LMIC.opmode & OP_TRACK) { Serial.print(F("OP_TRACK ")); }
    if (LMIC.opmode & OP_JOINING) { Serial.print(F("OP_JOINING ")); }
    if (LMIC.opmode & OP_TXDATA) { Serial.print(F("OP_TXDATA ")); }
    if (LMIC.opmode & OP_POLL) { Serial.print(F("OP_POLL ")); }
    if (LMIC.opmode & OP_REJOIN) { Serial.print(F("OP_REJOIN ")); }
    if (LMIC.opmode & OP_SHUTDOWN) { Serial.print(F("OP_SHUTDOWN ")); }
    if (LMIC.opmode & OP_TXRXPEND) { Serial.print(F("OP_TXRXPEND ")); }
    if (LMIC.opmode & OP_RNDTX) { Serial.print(F("OP_RNDTX ")); }
    if (LMIC.opmode & OP_PINGINI) { Serial.print(F("OP_PINGINI ")); }
    if (LMIC.opmode & OP_PINGABLE) { Serial.print(F("OP_PINGABLE ")); }
    if (LMIC.opmode & OP_NEXTCHNL) { Serial.print(F("OP_NEXTCHNL ")); }
    if (LMIC.opmode & OP_LINKDEAD) { Serial.print(F("OP_LINKDEAD ")); }
    if (LMIC.opmode & OP_LINKDEAD) { Serial.print(F("OP_LINKDEAD ")); }
    if (LMIC.opmode & OP_TESTMODE) { Serial.print(F("OP_TESTMODE ")); }
    if (LMIC.opmode & OP_UNJOIN) { Serial.print(F("OP_UNJOIN ")); }
    Serial.println("");
}

void LoraWANDebug(lmic_t lmic_to_check)
{
    Serial.println("");
    Serial.println("");
    
    LoraWANPrintLMICOpmode();

    Serial.print("LMIC.seqnoUp = ");
    Serial.println(lmic_to_check.seqnoUp); 

    Serial.print("LMIC.globalDutyRate = ");
    Serial.print(lmic_to_check.globalDutyRate);
    Serial.print(" osTicks, ");
    Serial.print(osticks2ms(lmic_to_check.globalDutyRate)/1000);
    Serial.println(" sec");

    Serial.print("LMIC.globalDutyAvail = ");
    Serial.print(lmic_to_check.globalDutyAvail);
    Serial.print(" osTicks, ");
    Serial.print(osticks2ms(lmic_to_check.globalDutyAvail)/1000);
    Serial.println(" sec");

    Serial.print("LMICbandplan_nextTx = ");
    Serial.print(LMICbandplan_nextTx(os_getTime()));
    Serial.print(" osTicks, ");
    Serial.print(osticks2ms(LMICbandplan_nextTx(os_getTime()))/1000);
    Serial.println(" sec");

    Serial.print("os_getTime = ");
    Serial.print(os_getTime());
    Serial.print(" osTicks, ");
    Serial.print(osticks2ms(os_getTime()) / 1000);
    Serial.println(" sec");

    Serial.print("LMIC.txend = ");
    Serial.println(lmic_to_check.txend);
    Serial.print("LMIC.txChnl = ");
    Serial.println(lmic_to_check.txChnl);

    Serial.println("Band \tavail \t\tavail_sec\tlastchnl \ttxcap");
    for (u1_t bi = 0; bi < MAX_BANDS; bi++)
    {
        Serial.print(bi);
        Serial.print("\t");
        Serial.print(lmic_to_check.bands[bi].avail);
        Serial.print("\t\t");
        Serial.print(osticks2ms(lmic_to_check.bands[bi].avail)/1000);
        Serial.print("\t\t");
        Serial.print(lmic_to_check.bands[bi].lastchnl);
        Serial.print("\t\t");
        Serial.println(lmic_to_check.bands[bi].txcap);
        
    }
    Serial.println("");
    Serial.println("");
}