#ifndef DISABLE_CLI

#include <ESP32WifiCLI.hpp>
#include <wifi.hpp>
#include <Sensors.hpp>
#ifndef DISABLE_BATT
#include <Batterylib.hpp>
#endif
#include "logmem.hpp"
#include "Watchdog.hpp"
#include "power.hpp"

void cliInit();
void cliTaskInit();
int32_t cliTaskStackFree();

#endif