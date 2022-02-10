#ifndef Watchdog_hpp
#define Watchdog_hpp

#include <Arduino.h>
#include <esp_wifi.h>

/**
 * The timer watchdog check inactivity on loop, if after
 * WATCHDOG_TIME the timer don't be reset, 
 * the device going to reboot. Also please see the
 * FORCE_WATCHDOG flag on platformio.ini.
 */
#define WATCHDOG_TIME 120        // check each WATCHDOG_TIME in seconds

#ifdef FORCE_WATCHDOG
#define FORCE_WATCHDOG_TIME 15  // force reboot in minutes
#endif

class Watchdog 
{

  public:

   void init();

   void pause();

   void resume();

   void execute();

   void loop();

  private:
   
   // Watchdog timer
   hw_timer_t *timer = NULL;

   unsigned int resetvar = 0;

};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_WATCHDOGHANDLER)
extern Watchdog wd;
#endif

#endif