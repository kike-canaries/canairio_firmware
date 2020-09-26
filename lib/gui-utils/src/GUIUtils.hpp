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

    void displaySensorAverage(int average);

    void displaySensorData(int pm25, int pm10, int chargeLevel, float humi, float temp, int rssi);

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

    void displayCenterBig(String msg);

    void displayBottomLine(String msg);

    void displayEmoticonLabel(int numsmile, String msg);

    void displayBigEmoticon(String msg);

    void displayBigLabel(int cursor, String msg);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_GUIHANDLER)
extern GUIUtils gui;
#endif

#endif
