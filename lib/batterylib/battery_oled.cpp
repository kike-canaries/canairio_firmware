#include <battery_oled.hpp>
#ifndef DISABLE_BATT

#ifdef M5PICOD4
    #define ADC_PIN 36
    int channel_atten = ADC1_CHANNEL_6;
#elif TTGO_T7
    #define ADC_PIN 35
    int channel_atten = ADC1_CHANNEL_7;
#elif TTGO_T7S3
    #define ADC_PIN 2
    int channel_atten = ADC1_CHANNEL_1;
#elif ESP32C3_AIRGRADIENT
    #define ADC_PIN 4
    int channel_atten = ADC1_CHANNEL_1;
#elif ESP32C3LOLIN
    #define ADC_PIN 4
    //int channel_atten = ADC1_CHANNEL_4;
    #define ADC1_EXAMPLE_CHAN4          ADC1_CHANNEL_4 //ADC Channels
    static const char *TAG_CH[6][10] = {"ADC1_CH4"};
    #define ADC_EXAMPLE_ATTEN           ADC_ATTEN_DB_6 //ADC Attenuation
    #define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP //ADC Calibration
    static int adc_raw[2][10];
    static const char *TAG = "ADC SINGLE";
    static esp_adc_cal_characteristics_t adc1_chars;
  static bool adc_calibration_init(void)
    {
        esp_err_t ret;
        bool cali_enable = false;
        ret = esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME);
        if (ret == ESP_ERR_NOT_SUPPORTED) {
           Serial.println( "Calibration scheme not supported, skip software calibration");
        } else if (ret == ESP_ERR_INVALID_VERSION) {
            Serial.println("eFuse not burnt, skip software calibration");
        } else if (ret == ESP_OK) {
            cali_enable = true;
            esp_adc_cal_characterize(ADC_UNIT_1, ADC_EXAMPLE_ATTEN, ADC_WIDTH_BIT_12, 0, &adc1_chars);
        } else {
            Serial.println("Invalid arg");
        }

    return cali_enable;
  }

#else
    #define ADC_PIN 34
    int channel_atten = 0;
#endif

float pavg = 0.0;

void Battery_OLED::setupBattADC() {
#ifdef ESP32C3LOLIN
    esp_err_t ret = ESP_OK;
    uint32_t voltage = 0;
    bool cali_enable = adc_calibration_init();
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN4, ADC_EXAMPLE_ATTEN));
          
  #else
    // TODO: all here is deprecated we need review the documentation
    esp_adc_cal_characteristics_t adc_chars;
   esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)channel_atten, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
   analogReadResolution(12);
   // Check type of calibration value used to characterize ADC
   if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
      log_i("[BATT] ADC eFuse Vref  \t: %u mV\r\n", adc_chars.vref);
      vref = adc_chars.vref;
      }  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
      log_i("[BATT] ADC Two Point coeff_a\t: %umV coeff_b:%umV\r\n", adc_chars.coeff_a, adc_chars.coeff_b);
      } else {
          log_i("[BATT] ADC Default Vref  \t: %u mV\r\n", vref);
    }
  #endif
}

void Battery_OLED::init(bool debug) {
  this->debug = debug;
  /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */
  #ifndef TTGO_T7S3
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);
  #endif
  delay(10);  // suggested by @ygator user in issue #2
  setupBattADC();
  delay(10);  // suggested by @ygator user in issue #2
  // setLimits(BATTERY_MIN_V, BATTERY_MAX_V, BATTCHARG_MIN_V, BATTCHARG_MAX_V);
}

float Battery_OLED::getVoltage() {
  return curv;
}

bool Battery_OLED::isCharging() {
  bool charging = false;
  if (isDischarging >= 0)
    charging = !isDischarging;
  else
    charging = curv > btDiscVMax + (btCharVMin - btDiscVMax) / 2;
//   if (debug) Serial.printf("-->[BATT] Batt is charging\t: %s\r\n", charging ? "True" : "False");
  return charging;
}

void Battery_OLED::printValues() {
  if (!debug) return;
  Serial.printf("-->[BATT] Battery voltage  \t: %.3fv vref: %i Charge:%i\r\n", curv, vref, getCharge());  // Output voltage and current of Bat
}

void Battery_OLED::update() {
  digitalWrite(ADC_EN, HIGH);
  delay(10);  // suggested by @ygator user in issue #2
  #ifdef ESP32C3LOLIN
while (1) {
        adc_raw[0][4] = adc1_get_raw(ADC1_EXAMPLE_CHAN4);
         
      //  if (cali_enable) {
            uint16_t v = esp_adc_cal_raw_to_voltage(adc_raw[0][4], &adc1_chars);
            Serial.print("channel 4=  ");
            Serial.println(v);   
       // }
        // delay(1000);
        }
       // curv = (v / 4095.0) * 15.83;
  #endif
  uint16_t v = analogRead(ADC_PIN);
#if defined(TTGO_T7) || defined(TTGO_T7S3)
  curv = ((float)v / 4095.0) * 7.58;
#else
  curv = ((float)v / 4095.0) * 15.83;
#endif
  if (!captureStage) digitalWrite(ADC_EN, LOW);  // for possible issue: https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/6
}

int Battery_OLED::getCharge() {
    if (isCharging()) {
        return calcPercentage(curv, btCharVMax, btCharVMin);
    } else {
        return calcPercentage(curv, btDiscVMax, btDiscVMin);
    }
}
#endif
#ifdef DISABLE_BATT
void init(bool debug = false) {}
float getVoltage() { return 0.0; }
bool isCharging() { return false; }
int getCharge()   { return 0; }
void printValues() {}
void update() {}
#endif

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OLEDBATTERY)
    #ifndef M5STICKCPLUS
        #ifndef TTGO_TDISPLAY
        Battery_OLED battery;
        #endif
    #endif
#endif
