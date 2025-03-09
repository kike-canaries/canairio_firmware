#include <WiFi.h>
#include <Sensors.hpp>
#include <Batterylib.hpp>
#include "wifi.hpp"
#include "ConfigApp.hpp"

#define IFX_RETRY_CONNECTION      5   // influxdb publish retry 
#define IFX_ERROR_COUNT_MAX       5   // max error count before full ESP restart 

void influxDbInit();
void influxDbLoop();