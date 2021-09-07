#ifndef GUIUtils_hpp
#define GUIUtils_hpp

#include <U8g2lib.h>
#include "hal.hpp"

class GUIUserPreferencesCallbacks; 
class GUIUtils {
   public:
    GUIUtils(void){};

    void displayInit();

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
    
    void setSensorData(int mainValue, int chargeLevel, float humi, float temp, int rssi, int deviceType);

    void setGUIStatusFlags(bool wifiOn, bool bleOn, bool blePair);

    void setBrightness(uint32_t value);

    void setWifiMode(bool enable);

    void setSampleTime(int time);

    void setTrackValues(float speed, float distance);

    void setTrackTime(int h, int m, int s);

    void suspendTaskGUI();

    void resumeTaskGUI();

    String getFirmwareVersionCode ();

   private:

    int lastDrawedLine = 0;

    bool dataOn;

    bool preferenceSave;

    bool sensorLive;

    U8G2 u8g2;

    int dw = 0;  // display width

    int dh = 0;  // display height

    int _rssi = 0;

    int _deviceType = 0;

    float _humi = 0.0;

    float _temp = 0.0;

    int _mainValue = 0;

    int _average = 0;

    bool _wifi_enable;

    bool _wifiOn;

    bool _bleOn;

    bool _blePair;

    bool isNewData;

    TaskHandle_t xHandle;

    bool taskGUIrunning;

    void displaySensorAverage(int average);

    void displayCenterBig(String msg);

    void displayEmoticonLabel(int numsmile, String msg);

    void displayBigEmoticon(String msg);

    void displayBigLabel(int cursor, String msg);

    void setupGUITask();

};

class GUIUserPreferencesCallbacks {
public:
    virtual ~GUIUserPreferencesCallbacks () {};
    virtual void onWifiMode(bool enable);
	virtual void onBrightness(int value);
    virtual void onColorsInverted(bool enable);
    virtual void onSampleTime(int time);
    virtual void onCalibrationReady();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_GUIHANDLER)
extern GUIUtils gui;
#endif

#endif
