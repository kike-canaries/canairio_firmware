#include <Watchdog.hpp>

void Watchdog::loop() {
    static uint_fast64_t timeStamp = 0;  // timestamp for loop check
    if ((millis() - timeStamp > 1000)) {
        timeStamp = millis();
        timerWrite(timer, 0);  //reset timer (feed watchdog)
#ifdef FORCE_WATCHDOG
        if (resetvar++ == FORCE_WATCHDOG_TIME * 60) {
            resetvar = 0;
            Serial.print("-->[WDOG] Watchdog running force reboot in ");
            Serial.print(WATCHDOG_TIME);
            Serial.println(" seconds.");
            delay(WATCHDOG_TIME * 2 * 1000);  // force to do watchdog
        }
#endif
    }
}

void IRAM_ATTR resetModule() {
    Serial.println("-->[WDOG] Watchdog reached, rebooting..");
    digitalWrite(MAIN_HW_EN_PIN, LOW);
    esp_wifi_disconnect();
    delay(200);
    esp_wifi_stop();
    delay(200);
    esp_wifi_deinit();
    digitalWrite(MAIN_HW_EN_PIN, HIGH);
    delay(200);
    ESP.restart();
}

void Watchdog::init() {
    timer = timerBegin(0, 80, true);                         // timer 0, div 80
    timerAttachInterrupt(timer, &resetModule, true);         // setting callback
    timerAlarmWrite(timer, WATCHDOG_TIME * 1000000, false);  // set time in us
    timerAlarmEnable(timer);                                 // enable interrupt

    Serial.print("-->[WDOG] watchdog check each\t: ");
    Serial.print(WATCHDOG_TIME);
    Serial.println(" seconds.");
#ifdef FORCE_WATCHDOG
    Serial.print("-->[WDOG] watchdog force reboot each ");
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

void Watchdog::execute() {
    resetModule();
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_WATCHDOGHANDLER)
Watchdog wd;
#endif
