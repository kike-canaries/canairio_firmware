
#include <Arduino.h>
#include <esp_wifi.h>

/**
 * The timer watchdog check inactivity on loop, if after
 * WATCHDOG_TIME the timer don't be reset, 
 * the device going to reboot. Also please see the
 * FORCE_WATCHDOG flag on platformio.ini.
 */
#define WATCHDOG_TIME 30  // check each WATCHDOG_TIME in seconds

#ifdef FORCE_WATCHDOG
#define FORCE_WATCHDOG_TIME 2  // force reboot in minutes
#endif

void watchdogInit();

void watchdogLoop();
