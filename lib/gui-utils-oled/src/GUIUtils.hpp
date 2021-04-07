#ifndef GUIUtils_hpp
#define GUIUtils_hpp

#include <U8g2lib.h>

class GUIUtils {
   public:
    GUIUtils(void){};

    void displayInit();

    void showWelcome();

    void showProgress(unsigned int progress, unsigned int total);

    void welcomeAddMessage(String msg);
    
    void welcomeRepeatMessage(String msg);

    void displaySensorAverage(int average, int deviceType);

    void displaySensorData(int mainValue, int chargeLevel, float humi, float temp, int rssi, int deviceType);

    void displayStatus(bool wifiOn, bool bleOn, bool blePair);

    void displayDataOnIcon();

    void displaySensorLiveIcon();

    void displayPreferenceSaveIcon();

    void pageStart();

    void pageEnd();

    String getFirmwareVersionCode ();

   private:

    int lastDrawedLine = 0;

    bool dataOn;

    bool preferenceSave;

    bool sensorLive;

    U8G2 u8g2;

    int dw = 0;  // display width

    int dh = 0;  // display height

    void displayCenterBig(String msg, int deviceType);

    void displayBottomLine(String msg);

    void displayEmoticonLabel(int numsmile, String msg);

    void displayBigEmoticon(String msg);

    void displayBigLabel(int cursor, String msg);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_GUIHANDLER)
extern GUIUtils gui;
#endif

#endif
