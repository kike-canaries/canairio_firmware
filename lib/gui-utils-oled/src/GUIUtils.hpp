#ifndef GUIUtils_hpp
#define GUIUtils_hpp

#include <FS.h>
#include <U8g2lib.h>

enum AQI_COLOR { AQI_NONE, AQI_PM, AQI_CO2 };

typedef struct GUIData {
    uint8_t mainUnitId;
    uint8_t onSelectionUnit;
    uint16_t mainValue;
    uint16_t minorValue;
    String unitName;
    String unitSymbol;
    float humi;
    float temp;
    int rssi;
    AQI_COLOR color;
} gdata;

class GUIUserPreferencesCallbacks; 
class GUIUtils {
   public:
    GUIUtils(void){};

    void displayInit(int ssd1306_type = 0);

    void setCallbacks(GUIUserPreferencesCallbacks* pCallbacks);

    void showWelcome();

    void showMain();

    void showProgress(unsigned int progress, unsigned int total);

    void welcomeAddMessage(String msg);
    
    void welcomeRepeatMessage(String msg);

    void displayGUIStatusFlags();

    void displayMainValues();

    void displayDataOnIcon();

    void displaySensorLiveIcon();

    void displayPreferenceSaveIcon();

    void displayBottomLine(String msg);
    
    void pageStart();

    void pageEnd();

    void clearScreen();

    void setSensorData(GUIData data);

    void setGUIStatusFlags(bool wifiOn, bool bleOn, bool blePair);

    void setInfoData(String info);

    void setBatteryStatus(float volts, int charge, bool isCharging);

    void setBrightness(uint32_t value);

    void setWifiMode(bool enable);

    void setPaxMode(bool enable);

    void setSampleTime(int time);

    void setTrackValues(float speed, float distance);

    void setTrackTime(int h, int m, int s);

    void setEmoticons(bool enable);

    void flipVertical (bool enable);

    void suspendTaskGUI();

    void resumeTaskGUI();

    int32_t getStackFree();

    void setPowerSave();

    String getFirmwareVersionCode ();

    void loop();

   private:

    int lastDrawedLine = 0;

    bool dataOn;

    bool preferenceSave;

    bool sensorLive;

    U8G2 *u8g2;

    int dw = 0;  // display width

    int dh = 0;  // display height

    int _rssi = 0;

    int _deviceType = 0;

    float _humi = 0.0;

    float _temp = 0.0;

    int _mainValue = 0;

    int _minorValue = 0;

    String _unit_symbol = "";

    String _unit_name = "";

    int _unit = 0;

    int _average = 0;

    bool _wifi_enable;

    bool _wifiOn;

    bool _bleOn;

    bool _blePair;

    float _batteryVolts;

    int _batteryCharge;

    bool _isCharging;

    bool isNewData;

    bool emoticons;

    TaskHandle_t xHandle;

    bool taskGUIrunning;

    void displaySensorAverage(int average);

    void displayAQIColor(int average);

    void displayCenterBig(String msg);

    void displayEmoticonLabel(int numsmile, String msg);

    void displayBigEmoticon(String msg);

    void displayBigLabel(int cursor, String msg);

    void displayWifiIcon();

    void displayBatteryIcon();

    void displayStatusBar();
    
    void displayUnit();

    void setupGUITask();

};

class GUIUserPreferencesCallbacks {
public:
    virtual ~GUIUserPreferencesCallbacks () {};
    virtual void onWifiMode(bool enable);
    virtual void onPaxMode(bool enable);
    virtual void onBrightness(int value);
    virtual void onColorsInverted(bool enable);
    virtual void onSampleTime(int time);
    virtual void onCalibrationReady();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_GUIHANDLER)
extern GUIUtils gui;
#endif

#endif
