#include <watchdog.hpp>

// Watchdog timer
hw_timer_t *timer = NULL;
unsigned int resetvar = 0;

/******************************************************************************
*   R E S E T
******************************************************************************/

void IRAM_ATTR resetModule() {
    Serial.println("\n-->[INFO] Watchdog reached, rebooting..");
    esp_wifi_disconnect();
    delay(200);
    esp_wifi_stop();
    delay(200);
    esp_wifi_deinit();
    delay(200);
    ESP.restart();
}

void watchdogLoop() {
    timerWrite(timer, 0);  //reset timer (feed watchdog)
    if (resetvar == 1199) {
        resetvar = 0;
        delay(45000);  // 45 seconds, reset at 30 seconds
    }
    resetvar++;
}

void watchdogInit() {
    timer = timerBegin(0, 80, true);                  // timer 0, div 80
    timerAttachInterrupt(timer, &resetModule, true);  // setting callback
    timerAlarmWrite(timer, 30000000, false);          // set time in us (30s)
    timerAlarmEnable(timer);                          // enable interrupt
}
