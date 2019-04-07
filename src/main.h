
using namespace std;

HardwareSerial hpmaSerial(1);
HPMA115S0 hpma115S0(hpmaSerial);

vector<unsigned int> v25;      // for average
vector<unsigned int> v10;      // for average
unsigned int apm25 = 0;        // last PM2.5 average
unsigned int apm10 = 0;        // last PM10 average
int stime = 5;                 // sample time (send data each 5 sec)
double lat,lon;                // Coordinates
float alt, spd;                // Altitude and speed
#define SENSOR_RETRY  1000     // Sensor read retry

// Humidity sensor
Adafruit_AM2320 am2320 = Adafruit_AM2320();
float humi = 0.0;              // % Relative humidity 
float temp = 0.0;              // Temperature (C)

// WiFi fields
#define WIFI_RETRY_CONNECTION    20
String ssid, pass;
bool dataSendToggle;
bool wifiEnable, wifiOn;
bool isNewWifi;
uint64_t chipid;

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
CanAirIoApi api(true);

// InfluxDB fields
#define IFX_RETRY_CONNECTION   5
InfluxArduino influx;
String ifxdb, ifxip, ifxuser, ifxpassw, ifxid, ifxtg, ifusr, ifpss, ifcer;
uint16_t ifxpt;
bool isNewIfxdbConfig;

// GUI fields
#define LED 2
GUIUtils gui;

// Config Settings
Preferences preferences;
const char app_name[] = "canairio";

// some prototypes
bool wifiCheck();
void wifiConnect(const char* ssid, const char* pass);
void wifiInit();
void wifiStop();
void wifiRestart();
void wifiLoop();

void influxDbReconnect();
void reboot();