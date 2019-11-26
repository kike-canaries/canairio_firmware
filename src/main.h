
using namespace std;

HardwareSerial hpmaSerial(1);
HPMA115S0 hpma115S0(hpmaSerial);

vector<unsigned int> v25;      // for average
vector<unsigned int> v10;      // for average
unsigned int apm25 = 0;        // last PM2.5 average
unsigned int apm10 = 0;        // last PM10 average
#define SENSOR_RETRY  1000     // Sensor read retry

// Humidity sensor
Adafruit_AM2320 am2320 = Adafruit_AM2320();
float humi = 0.0;              // % Relative humidity 
float temp = 0.0;              // Temperature (C)

#ifdef TTGO_TQ
// Battery level
const int IP5306_2 = 27;     // PIN2 IP5306
const int IP5306_3 = 14;     // PIN3 IP5306
unsigned int chargeLevel = 0;
unsigned int Rdelay = 0;
unsigned int resetvar = 0;
#endif

// WiFi fields
#define WIFI_RETRY_CONNECTION    20
bool dataSendToggle;
bool wifiOn;

// Bluetooth fields
BLEServer* pServer = NULL;
BLECharacteristic* pCharactData = NULL;
BLECharacteristic* pCharactConfig = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
#define SERVICE_UUID        "c8d1d262-861f-4082-947e-f383a259aaf3"
#define CHARAC_DATA_UUID    "b0f332a8-a5aa-4f3f-bb43-f99e7791ae01"
#define CHARAC_CONFIG_UUID  "b0f332a8-a5aa-4f3f-bb43-f99e7791ae02"

// CanAirIO API fields
CanAirIoApi api(false);

// InfluxDB fields
#define IFX_RETRY_CONNECTION   5
InfluxArduino influx;

// Config and settings handler
ConfigApp cfg;
int triggerSaveIcon = 0;

// GUI fields
#define LED 2
GUIUtils gui;

// Watchdog timer
hw_timer_t *timer = NULL;

// some prototypes
bool wifiCheck();
void wifiConnect(const char* ssid, const char* pass);
void wifiInit();
void wifiStop();
void wifiRestart();
void wifiLoop();