#include <Arduino.h>

#define BATTERY_LOOP_INTERVAL   1

void batteryInit();

void batteryloop();

unsigned int getChargeLevel();