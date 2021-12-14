
#include <battery.hpp>

/******************************************************************************
*   B A T T E R Y   C H A R G E   S T A T U S   M E T H O D S
******************************************************************************/

int vref = 931; //1100
float curv = 0;

// Battery level
unsigned int chargeLevel = 0;

#ifdef TTGO_TQ
const int IP5306_2 = 27;  // PIN2 IP5306
const int IP5306_3 = 14;  // PIN3 IP5306
unsigned int Rdelay = 0;
      
#endif

void batteryInit() {
#ifdef TTGO_TQ
    pinMode(IP5306_2, INPUT);
    pinMode(IP5306_3, INPUT);
    #else
    pinMode(ADC_PIN, INPUT);
    pinMode(ADC_EN, OUTPUT);
    digitalWrite(ADC_EN, HIGH);
    delay(10); 
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("-->[BATT] ADC eFuse Vref:%u mV\n", adc_chars.vref);
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("-->[BATT] ADC Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
    } else {
        Serial.printf("-->[BATT] ADC Default Vref: %u mV\n", vref);
    }
#endif
}

void batteryloop() {
#ifdef TTGO_TQ
    static uint_fast64_t timeStamp = 0;  // timestamp for loop check
    if ((millis() - timeStamp > BATTERY_LOOP_INTERVAL*1000)) {
        timeStamp = millis();
        Rdelay = 0;
        while (digitalRead(IP5306_2) == HIGH) {
            delayMicroseconds(100);  // Sincronization in 1
        }
        delayMicroseconds(50);  // Probably double shoot in 0
        while (digitalRead(IP5306_2) == HIGH) {
            delayMicroseconds(100);  // Sincronization in 1
        }
        while (digitalRead(IP5306_2) == LOW && Rdelay < 56) {
            delayMicroseconds(100);  // Sincronization in 0
            Rdelay = Rdelay + 1;
        }
        if (Rdelay > 52) {
            chargeLevel = 0;  // 0%
            return;
        }
        delayMicroseconds(1600);
        if (digitalRead(IP5306_2) == HIGH) {
            delayMicroseconds(100);
            if (digitalRead(IP5306_2) == HIGH) {
                chargeLevel = 100;  // 100%
                return;
            }
        }
        if (digitalRead(IP5306_3) == LOW) {
            delayMicroseconds(100);
            if (digitalRead(IP5306_3) == LOW) {
                chargeLevel = 25;  // 25%
                return;
            }
        }
        delayMicroseconds(1100);
        if (digitalRead(IP5306_3) == HIGH) {
            delayMicroseconds(100);
            if (digitalRead(IP5306_3) == HIGH) {
                chargeLevel = 75;  // 75%
                return;
            }
        }
        if (digitalRead(IP5306_3) == LOW) {
            chargeLevel = 50;  // 50%
            return;
        }
    }
    #else
    digitalWrite(ADC_EN, HIGH);
    delay(10);                         // suggested by @ygator user in issue #2
    uint16_t v = analogRead(ADC_PIN);
    curv = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    digitalWrite(ADC_EN, LOW);   // for possible issue: https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/6
    // return curv;
#endif
}

unsigned int getChargeLevel() {
#ifdef TTGO_TQ
    Serial.print("-->[BATT] chargelevel: ");
    Serial.print(chargeLevel);
    Serial.println("%");
#endif
return chargeLevel;
} 