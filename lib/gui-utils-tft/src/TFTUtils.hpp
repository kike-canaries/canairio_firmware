#ifndef TFTUtils_hpp
#define TFTUtils_hpp

#include <SPI.h>
#include <TFT_eSPI.h>  // Hardware-specific library
// #include "ani.h"
#include "hal.hpp"
#include "battery.hpp"
#include "Orbitron_Medium_20.h"
#include "icons.h"

// Graph bars definition
#define MAX_X 135
#define MAX_Y 80
// Setup screen
#define SSTART 75
#define MARGINL 5
#define MARVALL 74
#define PRESETH 20

#define TFT_GREY 0x5AEB
#define lightblue 0x01E9
#define darkred 0xA041
#define blue 0x5D9B


class GUIUserPreferencesCallbacks; 
class TFTUtils {
   public:
    TFTUtils(void){};

    TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

    enum WIFI_MODE { WIFI_OFF, WIFI_ON };

    void displayInit();

    void setCallbacks(GUIUserPreferencesCallbacks* pCallbacks);

    void showWelcome();

    void showMain();

    void showProgress(unsigned int progress, unsigned int total);

    void welcomeAddMessage(String msg);
    
    void welcomeRepeatMessage(String msg);

    void displaySensorData(int mainValue, int chargeLevel, float humi, float temp, int rssi, int deviceType);

    void displayStatus(bool wifiOn, bool bleOn, bool blePair);

    void displayDataOnIcon();

    void displaySensorLiveIcon();

    void displayPreferenceSaveIcon();

    void displayBottomLine(String msg);

    void pageStart();

    void pageEnd();

    void clearScreen();

    void setBrightness(uint32_t value);

    void setWifiMode(bool enable);

    void setSampleTime(int time);

    void checkButtons();

    void suspend();

    String getFirmwareVersionCode ();

   private:

    const int pwmFreq = 5000;

    const int pwmResolution = 8;

    const int pwmLedChannelTFT = 0;

    int backlight[5] = {10, 30, 60, 120, 220};

    byte b = 1;  // backlight selector

    int brightness = 30;

    int sampleTime[5] = {5, 15, 30, 60, 120};

    byte st = 0;  // sample time selector init

    uint32_t count = 0;

    bool inv = 1;

    int lastDrawedLine = 0;

    bool dataOn;

    bool preferenceSave;

    bool sensorLive;

    bool fanState;

    float speed;

    float km;

    int hours;

    int minutes;

    int dw = 0;  // display width

    int dh = 0;  // display height

    uint32_t pkts[MAX_X];  // here the packets per second will be saved

    int state = 0;

    int press1 = 0;

    int press2 = 0;

    int wstate = 0;

    int _live_ticks = 0;

    int _rssi = 0;

    int _deviceType = 0;

    float _humi = 0.0;

    float _temp = 0.0;

    int _mainValue = 0;

    int _average = 0;

    bool _wifi_enable;

    int _sample_time = 5;

    void showStatus();

    void showSetup();

    void refreshSetup();

    void displaySensorAverage(int average);

    void displayMainUnit(String unit);

    void displayCenterBig(String msg);

    void displayEmoticonLabel(int numsmile, String msg);

    void displayEmoticonColor(uint32_t color, String msg);

    void displayBigEmoticon(String msg);

    void displayBigLabel(int cursor, String msg);

    double getMultiplicator();

    uint32_t getAQIColor(uint32_t value,int deviceType);
    
    void drawBarGraph(int deviceType);

    void drawBluetoothIcon();

    void drawWifiHighIcon();

    void drawWifiMidIcon();

    void drawWifiLowIcon();

    void drawFanIcon();

    void drawDataIcon();

    void invertScreen();

    void updateInvertValue();

    void updateBrightness();

    void loadBrightness();
    
    void updateBatteryValue();

    void notifyWifiMode();

    void updateWifiMode();

    void notifySampleTime();

    void updateSampleTime();

    void notifyBrightness();

    void toggleWindow();

    void restoreMain();

    void loadLastData();

    void showWindowBike();

    GUIUserPreferencesCallbacks* mGUICallBacks = nullptr;

    TFTUtils* getInstance();

};

class GUIUserPreferencesCallbacks {
public:
    virtual ~GUIUserPreferencesCallbacks () {};
    virtual void onWifiMode(bool enable);
	virtual void onBrightness(int value);
    virtual void onColorsInverted(bool enable);
    virtual void onSampleTime(int time);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TFTHANDLER)
extern TFTUtils gui;
#endif

#endif
