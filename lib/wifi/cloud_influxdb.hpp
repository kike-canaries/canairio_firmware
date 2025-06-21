#include <Sensors.hpp>
#include <wifi.hpp>
#include <ConfigApp.hpp>

#define IFX_RETRY_CONNECTION      5   // influxdb publish retry 
#define IFX_ERROR_COUNT_MAX       5   // max error count before full ESP restart 
#define IFX_RETRY_PUBLISH         3   // retry publish to influxdb

void influxDbInit();
void influxDbLoop();