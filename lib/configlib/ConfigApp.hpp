#ifndef CanfigApp_hpp
#define CanfigApp_hpp

#include <ArduinoJson.h>
#include <Geohash.hpp>
#include <EasyPreferences.hpp>

class RemoteConfigCallbacks;

typedef struct trackStatus {
  float kms = 0.0;
  float spd = 0.0;
  int hrs = 0;
  int min = 0;
  int seg = 0;
} trackStatus;

extern trackStatus track;

typedef struct ifxdbValues {
  String db = "canairio";
  String ip = "influxdb.canair.io";
  uint16_t pt = 8086;
} ifxdbValues;

extern ifxdbValues ifx;

extern uint64_t chipid;
extern String deviceId;
extern String dname;

extern int stime;
extern int stype;
extern int sTX;
extern int sRX;
extern double lat;
extern double lon;
extern String geo;

extern String ssid;
extern String pass;

extern String hassip;
extern String hassusr;
extern String hasspsw;
extern int16_t hasspt;

extern bool isNewWifi;
extern bool devmode;
extern bool i2conly;
extern bool pax_enable; 
extern bool solarmode;
extern uint32_t deepSleep;
extern float toffset;
extern float altoffset;
extern float sealevel;

/// preferences main key
extern char* _app_name;
/// device wifi on/off
extern bool wifi_enable;
/// InfluxDB cloud publication on/off
extern bool ifxdb_enable;
/// WiFi state
extern bool wifi_connected;

extern Geohash geohash;

extern RemoteConfigCallbacks* mRemoteConfigCallBacks;

void init(const char app_name[]);
void reload();
bool save(const char* json);
// bool saveDeviceName(String name);
bool saveSampleTime(int time);
bool saveSensorType(int type);
bool saveSensorPins(int tx, int rx);
bool saveUnitSelected(int unit);
int getUnitSelected();
bool saveSSID(String ssid);
bool saveWifi(String ssid, String pass);
bool saveInfluxDb(String db, String ip, int pt);
bool saveGeo(String geo);
bool saveGeo(double lat, double lon, String geo);
bool wifiEnable(bool enable);
bool ifxdbEnable(bool enable);
bool debugEnable(bool enable);
bool paxEnable(bool enable);
bool solarEnable(bool enable);
bool saveDeepSleep(int seconds);
bool saveHassIP(String ip);
bool saveHassPort(int port);
bool saveHassPassword(String pass);
bool saveHassUser(String user);
String getCurrentConfig();
bool isWifiEnable();
bool isPaxEnable();
bool isIfxEnable();
void setWifiConnected(bool connected);
bool isWifiConnected();
String getDeviceId();
String getDeviceIdShort();
String getStationName();
String getAnaireDeviceId();
String getVersion();
int getSensorType();
// void clear();
// void reboot();
void saveBrightness(int value);
int32_t getBrightness();
void colorsInvertedEnable(bool enable);
bool getTrackStatusValues(const char* json);
bool saveTempOffset(float offset);
bool saveAltitudeOffset(float offset);
bool saveSeaLevel(float hpa);
void setRemoteConfigCallbacks(RemoteConfigCallbacks* pCallbacks);

// void setLastKeySaved(String key);
bool saveI2COnly(bool enable);
void performCO2Calibration();
// void DEBUG(const char* text, const char* textb = "");

class RemoteConfigCallbacks {
public:
    virtual ~RemoteConfigCallbacks () {};
    virtual void onCO2Calibration();
    virtual void onAltitudeOffset(float altitude);
    virtual void onSeaLevelPressure(float hpa);
};

#endif