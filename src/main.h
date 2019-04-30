
using namespace std;

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C

float hum_weighting = 0.25; // so hum effect is 25% of the total air quality score
float gas_weighting = 0.75; // so gas effect is 75% of the total air quality score

float hum_score, gas_score;
float gas_reference = 250000;
float hum_reference = 40;
int getgasreference_count = 0;
uint32_t aGAS;
float aAQS;
float aIAQ;
float prs;
float alt;
float hum;
float tmp;

vector<uint32_t> vGAS;   // GAS for average
vector<float> vAQS;      // AQS for average
vector<float> vIAQ;      // IAQ for average
#define SENSOR_RETRY  1000     // Sensor read retry

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

// GUI fields
#define LED 2
GUIUtils gui;

// some prototypes
bool wifiCheck();
void wifiConnect(const char* ssid, const char* pass);
void wifiInit();
void wifiStop();
void wifiRestart();
void wifiLoop();