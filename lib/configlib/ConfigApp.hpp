#ifndef CanfigApp_hpp
#define CanfigApp_hpp

#include <ArduinoJson.h>
#include <Preferences.h>

class ConfigApp {
   private:
    // Config Settings
    char* _app_name;
    Preferences preferences;
    String lastKeySaved = "";
    bool wifi_enable;
    bool ifxdb_enable;
    bool api_enable;

    void saveString(String key, String value);
    
    void saveInt(String key, int value);

    void saveBool(String key, bool value);

    void setLastKeySaved(String key);

   public:
    uint64_t chipid;
    char* deviceId;
    String dname;
    int stime;
    int stype;
    double lat;
    double lon;
    float alt;
    float spd;

    String ssid;
    String pass;

    struct ifxdbValues {
        String db;
        String ip;
        uint16_t pt = 8086;
    } ifx;

    String apiusr;
    String apipss;
    String apisrv;
    String apiuri;
    int apiprt;

    bool isNewIfxdbConfig;
    bool isNewAPIConfig;
    bool isNewWifi;

    void init(const char app_name[]);

    void reload();

    bool save(const char* json);

    bool saveDeviceName(String name);

    bool saveSampleTime(int time);

    bool saveSensorType(int type);
    
    bool saveWifi(String ssid, String pass);

    bool saveInfluxDb(String db, String ip, int pt);
    
    bool saveAPI(String usr, String pass, String srv, String uri, int pt);

    bool saveGeo(double lat, double lon, float alt, float spd);

    bool wifiEnable(bool enable);

    bool ifxdbEnable(bool enable);

    bool apiEnable(bool enable);

    String getCurrentConfig();

    bool isWifiEnable();

    bool isIfxEnable();

    bool isApiEnable();

    void clear();

    void reboot();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
extern ConfigApp cfg;
#endif

#endif