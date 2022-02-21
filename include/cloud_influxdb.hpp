#define IFX_MIN_PUBLISH_INTERVAL 30   // time in seconds
#define IFX_RETRY_CONNECTION      5   // influxdb publish retry 
#define IFX_ERROR_COUNT_MAX       5   // max error count before full ESP restart 
#define WAIT_FOR_SENSOR          25   // seconds

void influxDbInit();
void influxDbLoop();

