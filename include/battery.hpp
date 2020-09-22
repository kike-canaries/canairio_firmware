#include <Arduino.h>

#define BATTERY_LOOP_INTERVAL   10000

void batteryInit();

void batteryloop();

unsigned int getChargeLevel();