#ifndef DISABLE_CLI
#include <ESP32WifiCLI.hpp>
#include <Sensors.hpp>
#include "Batterylib.hpp"
#include "logmem.hpp"
#include "Watchdog.hpp"

void cliInit();
void cliTaskInit();
int32_t cliTaskStackFree();

#endif