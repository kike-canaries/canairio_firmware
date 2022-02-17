#include <power.hpp>

void prepairShutdown() {
#ifdef TTGO_TDISPLAY
    digitalWrite(ADC_EN, LOW);
    delay(10);
    rtc_gpio_init(GPIO_NUM_14);
    rtc_gpio_set_direction(GPIO_NUM_14, RTC_GPIO_MODE_OUTPUT_ONLY);
	rtc_gpio_set_level(GPIO_NUM_14, 1);
    delay(500); 
 
#else
   gui.setPowerSave(); 
#endif

}

void completeShutdown(){
    esp_bluedroid_disable();
    esp_bt_controller_disable();
    esp_wifi_stop();
    esp_deep_sleep_disable_rom_logging();
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_deep_sleep_start();
}

void powerDeepSleepButton(){
    prepairShutdown();
    //Disable timer wake, because here use external IO port to wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    completeShutdown();
}

void powerDeepSleepTimer(int seconds) {
    prepairShutdown();
    esp_sleep_enable_timer_wakeup(seconds * 1000000);
    #ifdef TTGO_TDISPLAY
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    #else
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 1);
    #endif
    completeShutdown(); 
}

void powerLightSleepTimer(int seconds) {
    esp_sleep_enable_timer_wakeup(seconds * 1000000);
    esp_light_sleep_start();
}

void powerInit() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable Brownout Detector
    if (battery.getVoltage() < 3.3) {
        Serial.println("-->[POWR] Goto DeepSleep (curv to low)");
        powerDeepSleepTimer(DEEP_SLEEP_TIME);
    }
    // set cpu speed low to save battery
    setCpuFrequencyMhz(80);
    Serial.print("-->[POWR] CPU Speed: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println(" MHz");
}
