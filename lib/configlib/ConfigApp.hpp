#ifndef CanfigApp_hpp
#define CanfigApp_hpp

#include <ArduinoJson.h>
#include <Preferences.h>
#include <Watchdog.hpp>
#include <Geohash.hpp>

#define RW_MODE false
#define RO_MODE true

typedef enum {
    INT, BOOL, FLOAT, STRING, UNKNOWN
} ConfKeyType;

#define CONFIG_KEYS_LIST              \
    X(KBWIFIEN, "wifiEnable", BOOL)   \
    X(KBPAXENB, "paxEnable", BOOL)    \
    X(KEMOTICO, "emoEnable", BOOL)    \
    X(KBI2COLY, "i2conly", BOOL)      \
    X(KFALTFST, "altoffset", FLOAT)   \
    X(KFTOFFST, "toffset", FLOAT)     \
    X(KBASIC, "-----", UNKNOWN)       \
    X(KBHOMEAS, "homeaEnable", BOOL)  \
    X(KBANAIRE, "anaireEnable", BOOL) \
    X(KBIFXENB, "ifxEnable", BOOL)    \
    X(KSIFXDB, "ifxdb", STRING)       \
    X(KSIFXIP, "ifxip", STRING)       \
    X(KIIFXPT, "ifxpt", INT)          \
    X(KSHASSU, "hassusr", STRING)     \
    X(KSHASSPW, "hasspsw", STRING)    \
    X(KIHASSPT, "hasspt", INT)        \
    X(KFSEALV, "sealevel", FLOAT)     \
    X(KWKUPRST, "wakeupByReset", BOOL)  \
    X(KBSOLARE, "solarEnable", BOOL)  \
    X(KIDEEPSL, "deepSleep", INT)     \
    X(KCOUNT, "KCOUNT", UNKNOWN)

#define X(kname, kreal, ktype) kname,
typedef enum CONFKEYS : size_t { CONFIG_KEYS_LIST } CONFKEYS; 
#undef X

class RemoteConfigCallbacks;
class ConfigApp {
   public:
    uint64_t chipid;
    String deviceId;
    String dname;
    
    int stime;
    int stype;
    int sTX;
    int sRX;
    double lat;
    double lon;
    String geo;

    String ssid;
    String pass;

    String hassip;
    String hassusr;
    String hasspsw;
    int16_t hasspt;
    
    struct ifxdbValues {
        String db = "canairio";
        String ip = "influxdb.canair.io";
        uint16_t pt = 8086;
    } ifx;

    bool isNewWifi;

    struct trackStatus {
        float kms = 0.0;
        float spd = 0.0;
        int hrs = 0;
        int min = 0;
        int seg = 0;
    } track;

    bool devmode;

    bool i2conly;

    bool pax_enable = true;

    bool solarmode = false;

    uint32_t deepSleep = 0;

    float toffset = 0.0;
    
    float altoffset = 0.0;

    float sealevel = 1013.25;

    void init(const char app_name[]);

    void reload();

    bool save(const char* json);

    bool saveDeviceName(String name);

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

    void saveInt(String key, int value);
    void saveInt(CONFKEYS key, int value);

    int32_t getInt(String key, int defaultValue);
    int32_t getInt(CONFKEYS key, int defaultValue);
    
    bool getBool(String key, bool defaultValue);
    bool getBool(CONFKEYS key, bool defaultValue);

    void saveBool(String key, bool value);
    void saveBool(CONFKEYS key, bool value);

    float getFloat(String key, float defaultValue);
    float getFloat(CONFKEYS key, float defaultValue);

    void saveFloat(String key, float value);
    void saveFloat(CONFKEYS key, float value);

    void saveString(String key, String value);
    void saveString(CONFKEYS key, String value);

    String getString(String key, String defaultValue);
    String getString(CONFKEYS key, String defaultValue);

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

    void clear();

    void reboot();

    void saveBrightness(int value);

    int32_t getBrightness();

    void colorsInvertedEnable(bool enable);

    bool getTrackStatusValues(const char *json);

    bool saveTempOffset(float offset);

    bool saveAltitudeOffset(float offset);

    bool saveSeaLevel(float hpa);

    void setRemoteConfigCallbacks(RemoteConfigCallbacks* pCallbacks);

    PreferenceType keyType(String key);

    bool isKey(String key);

    String getKey(CONFKEYS key);

    ConfKeyType getKeyType(String key);

    ConfKeyType getKeyType(CONFKEYS key);
    
   private: 
    ///preferences main key
    char* _app_name;
    ///ESP32 preferences abstraction
    Preferences preferences;
    ///last key saved (for callback)
    String lastKeySaved = "";
    ///device wifi on/off
    bool wifi_enable;
    ///InfluxDB cloud publication on/off
    bool ifxdb_enable; 
    ///WiFi state
    bool wifi_connected;

    Geohash geohash;

    RemoteConfigCallbacks* mRemoteConfigCallBacks = nullptr;
        
    void setLastKeySaved(String key);

    bool saveI2COnly(bool enable);

    void performCO2Calibration();

    void DEBUG(const char* text, const char* textb = "");

    // @todo use DEBUG_ESP_PORT ?
#ifdef WM_DEBUG_PORT
    Stream& _debugPort = WM_DEBUG_PORT;
#else
    Stream& _debugPort = Serial;  // debug output stream ref
#endif
};

class RemoteConfigCallbacks {
public:
    virtual ~RemoteConfigCallbacks () {};
    virtual void onCO2Calibration();
    virtual void onAltitudeOffset(float altitude);
    virtual void onSeaLevelPressure(float hpa);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
extern ConfigApp cfg;
#endif

#endif