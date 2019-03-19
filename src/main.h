
using namespace std;
/******************************************************************************
* S E T U P  B O A R D   A N D  F I E L D S
* ---------------------
* please select board on platformio.ini file
******************************************************************************/

#ifdef WEMOSOLED // display via i2c for WeMOS OLED board
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
#elif HELTEC // display via i2c for Heltec board
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 15, 4, 16);
#else       // display via i2c for D1MINI board
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE,U8X8_PIN_NONE,U8X8_PIN_NONE);
#endif

// HPMA115S0 sensor config
#ifdef WEMOSOLED
#define HPMA_RX 13   // config for Wemos board
#define HPMA_TX 15
#elif HELTEC
#define HPMA_RX 13  // config for Heltec board
#define HPMA_TX 12
#else
#define HPMA_RX 17  // config for D1MIN1 board
#define HPMA_TX 16
#endif

HardwareSerial hpmaSerial(1);
HPMA115S0 hpma115S0(hpmaSerial);

String txtMsg = "";
vector<unsigned int> v25;      // for average
vector<unsigned int> v10;      // for average
unsigned int apm25 = 0;        // last PM2.5 average
unsigned int apm10 = 0;        // last PM10 average
int stime = 5;                 // sample time (send data each 5 sec)
double lat,lon;                // Coordinates
float alt, spd;                // Altitude and speed

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

// InfluxDB fields
#define IFX_RETRY_CONNECTION   5
InfluxArduino influx;
String ifxdb, ifxip, ifxuser, ifxpassw, ifxid, ifxtg;
bool isNewIfxdbConfig;

// GUI fields
#define LED 2
GUIUtils gui;

// Config Settings
Preferences preferences;
const char app_name[] = "canairio";

// some prototypes
void wifiRestart();
bool wifiCheck();
void influxDbReconnect();
void reboot();