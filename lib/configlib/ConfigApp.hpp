#ifndef CanfigApp_hpp
#define CanfigApp_hpp

#include <ArduinoJson.h>
#include <Preferences.h>

class RemoteConfigCallbacks;
class ConfigApp {
   public:
    uint64_t chipid;
    String deviceId;
    String dname;
    int stime;
    int stype;
    double lat;
    double lon;
    String geo;

    String ssid;
    String pass;

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

    float toffset = 0.0;
    
    float altoffset = 0.0;

    void init(const char app_name[]);

    void reload();

    bool save(const char* json);

    bool saveDeviceName(String name);

    bool saveSampleTime(int time);

    bool saveSensorType(int type);

    bool saveWifi(String ssid, String pass);

    bool saveInfluxDb(String db, String ip, int pt);

    bool saveGeo(double lat, double lon, String geo);

    bool wifiEnable(bool enable);

    bool ifxdbEnable(bool enable);

    bool debugEnable(bool enable);

    String getCurrentConfig();

    bool isWifiEnable();

    bool isIfxEnable();

    void setWifiConnected(bool connected);

    bool isWifiConnected();

    String getDeviceId();

    String getDeviceIdShort();

    int getSensorType();

    void clear();

    void reboot();

    void saveBrightness(int value);

    int32_t getBrightness();

    void colorsInvertedEnable(bool enable);

    bool getTrackStatusValues(const char *json);

    bool saveTempOffset(float offset);

    bool saveAltitudeOffset(float offset);

    void setRemoteConfigCallbacks(RemoteConfigCallbacks* pCallbacks);

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

    RemoteConfigCallbacks* mRemoteConfigCallBacks = nullptr;
        
    void saveString(String key, String value);

    void saveInt(String key, int value);

    int32_t getInt(String key, int defaultValue);

    void saveFloat(String key, float value);

    void saveBool(String key, bool value);

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
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
extern ConfigApp cfg;
#endif

#endif