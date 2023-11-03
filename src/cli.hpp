#ifndef DISABLE_CLI

#include <ESP32WifiCLI.hpp>
#include <ConfigApp.hpp>
#include <logmem.hpp>

void cliInit();
void cliTaskInit();
int32_t cliTaskStackFree();

#endif