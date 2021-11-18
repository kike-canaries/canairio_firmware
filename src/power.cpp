#include <power.h>

void prepairShutdown() {
#ifdef TTGO_TDISPLAY
    digitalWrite(ADC_EN, LOW);
    delay(10);
    rtc_gpio_init(GPIO_NUM_14);
    rtc_gpio_set_direction(GPIO_NUM_14, RTC_GPIO_MODE_OUTPUT_ONLY);
	rtc_gpio_set_level(GPIO_NUM_14, 1);
    delay(500); 
 
#else
   gui.PowerSave(); 
#endif

}

void completeShutdown(){
    esp_bluedroid_disable();
    esp_bt_controller_disable();
    esp_wifi_stop();
    esp_deep_sleep_disable_rom_logging();
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
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    completeShutdown(); 
}

void powerLightSleepTimer(int seconds) {
    esp_sleep_enable_timer_wakeup(seconds * 1000000);
    esp_light_sleep_start();
}
