#include <Watchdog.hpp>

/******************************************************************************
*   R E S E T
******************************************************************************/

void Watchdog::loop() {
    static uint_fast64_t timeStamp = 0;  // timestamp for loop check
    if ((millis() - timeStamp > 1000)) {
        timeStamp = millis();
        timerWrite(timer, 0);  //reset timer (feed watchdog)
#ifdef FORCE_WATCHDOG
        if (resetvar++ == FORCE_WATCHDOG_TIME * 60) {
            resetvar = 0;
            Serial.print("-->[WATCHDOG] force reboot in ");
            Serial.print(WATCHDOG_TIME);
            Serial.println(" seconds.");
            delay(WATCHDOG_TIME * 2 * 1000);  // force to do watchdog
        }
#endif
    }
}

void IRAM_ATTR resetModule() {
    Serial.println("-->[WATCHDOG] Watchdog reached, rebooting..");
    esp_wifi_disconnect();
    delay(200);
    esp_wifi_stop();
    delay(200);
    esp_wifi_deinit();
    delay(200);
    ESP.restart();
}

void Watchdog::init() {
    timer = timerBegin(0, 80, true);                         // timer 0, div 80
    timerAttachInterrupt(timer, &resetModule, true);         // setting callback
    timerAlarmWrite(timer, WATCHDOG_TIME * 1000000, false);  // set time in us (30s)
    timerAlarmEnable(timer);                                 // enable interrupt

    Serial.print("-->[WATCHDOG] config to check each ");
    Serial.print(WATCHDOG_TIME);
    Serial.println(" seconds.");
#ifdef FORCE_WATCHDOG
    Serial.print("-->[WATCHDOG] force reboot each ");
    Serial.print(FORCE_WATCHDOG_TIME);
    Serial.println(" minutes.");
#endif
}

void Watchdog::pause() {
    timerAlarmDisable(timer);  // disable interrupt
}

void Watchdog::resume() {
    timerAlarmEnable(timer);  // enable interrupt
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_WATCHDOGHANDLER)
Watchdog wd;
#endif
