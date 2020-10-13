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

    void printError(const char * error);

    void saveString(String key, String value);
    
    void saveInt(String key, int value);

    void setLastKeySaved(String key);

   public:
    uint64_t chipid;
    char* deviceId;
    String dname;
    bool wifiEnable;
    bool ifxEnable;
    bool apiEnable;
    String ssid;
    String pass;
    String ifxdb;
    String ifxip;
    uint16_t ifxpt;
    String ifusr;
    String ifpss;
    String apiusr;
    String apipss;
    String apisrv;
    String apiuri;
    int apiprt;
    int stime;
    int stype;
    double lat;
    double lon;
    float alt;
    float spd;
    bool isNewIfxdbConfig;
    bool isNewAPIConfig;
    bool isNewWifi;

    void init(const char app_name[]);

    void reload();

    bool save(const char* json);

    bool saveDeviceName(String name);

    bool saveSampleTime(int time);

    String getCurrentConfig();

    bool isWifiEnable();

    bool isIfxEnable();

    bool isApiEnable();

    void reboot();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
extern ConfigApp cfg;
#endif

#endif