#ifndef TFTUtils_hpp
#define TFTUtils_hpp

#include <SPI.h>
#include <TFT_eSPI.h>  // Hardware-specific library
// #include "ani.h"

#define TFT_GREY 0x5AEB
#define lightblue 0x01E9
#define darkred 0xA041
#define blue 0x5D9B

#include "Orbitron_Medium_20.h"

class TFTUtils {
   public:
    TFTUtils(void){};

    TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

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

    const int pwmFreq = 5000;

    const int pwmResolution = 8;

    const int pwmLedChannelTFT = 0;

    int backlight[5] = {10, 30, 60, 120, 220};

    int lastDrawedLine = 0;

    bool dataOn;

    bool preferenceSave;

    bool sensorLive;

    int dw = 0;  // display width

    int dh = 0;  // display height

    void displayCenterBig(String msg, int deviceType);

    void displayBottomLine(String msg);

    void displayEmoticonLabel(int numsmile, String msg);

    void displayBigEmoticon(String msg);

    void displayBigLabel(int cursor, String msg);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TFTHANDLER)
extern TFTUtils gui;
#endif

#endif
