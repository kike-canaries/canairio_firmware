
/* ESP32C3 all 6 channels working including AC2 chan 0 during wifi
  single_read example by Claude Arpin CEO Seenov
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <Arduino.h>
#include "driver/adc_common.h"
#include "esp_adc_cal.h"


//ADC Channels

#define ADC1_EXAMPLE_CHAN4          ADC1_CHANNEL_4

static const char *TAG_CH[1][10] = { {"ADC1_CH4"} };
 

//ADC Attenuation
#define ADC_EXAMPLE_ATTEN           ADC_ATTEN_DB_6

//ADC Calibration

#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP



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


void setup() {
  
Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  delay(1000);
  Serial.println("starting");
}

void loop() {
 
esp_err_t ret = ESP_OK;
    uint32_t voltage = 0;
    bool cali_enable = adc_calibration_init();

    //ADC1 config
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));/*
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN1, ADC_EXAMPLE_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN2, ADC_EXAMPLE_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN3, ADC_EXAMPLE_ATTEN));*/
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN4, ADC_EXAMPLE_ATTEN));
    

    while (1) {
        adc_raw[0][4] = adc1_get_raw(ADC1_EXAMPLE_CHAN4);
         
        if (cali_enable) {
            voltage = esp_adc_cal_raw_to_voltage(adc_raw[0][4], &adc1_chars);
            Serial.print("channel 4=  ");
            Serial.println(voltage);   
        }
        delay(1000);
        }
        

}