#ifndef CanfigApp_hpp
#define CanfigApp_hpp

#include <ArduinoJson.h>
#include <Preferences.h>

class ConfigApp {
   public:
    uint64_t chipid;
    String deviceId;
    String dname;
    int stime;
    int stype;
    double lat;
    double lon;

    String ssid;
    String pass;

    struct ifxdbValues {
        String db = "canairio";
        String ip = "influxdb.canair.io";
        uint16_t pt = 8086;
    } ifx;

    bool isNewIfxdbConfig;

    bool isNewWifi;

    bool devmode;

    bool i2conly;

    float toffset = 0.0;

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

    int getSensorType();

    void clear();

    void reboot();

    void setDebugMode(bool enable);

    bool saveTempOffset(float offset);

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
        
    void saveString(String key, String value);
    void saveInt(String key, int value);
    void saveFloat(String key, float value);
    void saveBool(String key, bool value);
    void setLastKeySaved(String key);
    bool saveI2COnly(bool enable);
    void DEBUG(const char* text, const char* textb = "");

    // @todo use DEBUG_ESP_PORT ?
#ifdef WM_DEBUG_PORT
    Stream& _debugPort = WM_DEBUG_PORT;
#else
    Stream& _debugPort = Serial;  // debug output stream ref
#endif
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
extern ConfigApp cfg;
#endif

#endif