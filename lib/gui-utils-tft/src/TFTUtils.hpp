#ifndef TFTUtils_hpp
#define TFTUtils_hpp

#include <SPI.h>
#ifdef M5STICKCPLUS
#include <M5StickCPlus.h>
#else
#include <TFT_eSPI.h>
#endif
#include "hal.hpp"
#include <Orbitron_Medium_20.h>
#include "icons.h"

// Main windows
#define RCOLSTART 80

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
#define ligthgreen 0xF59F

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

    void setSensorData(GUIData data);

    void setGUIStatusFlags(bool wifiOn, bool bleOn, bool blePair);

    void setInfoData(String info);

    void displayMainValues();

    void displayDataOnIcon();

    void displaySensorLiveIcon();

    void displayPreferenceSaveIcon();

    void displayBottomLine(String msg);

    void pageStart();

    void checkButtons();

    void pageEnd();

    void clearScreen();

    void setBrightness(uint32_t value);

    void setWifiMode(bool enable);
    
    void setPaxMode(bool enable);

    void setSampleTime(int time);

    void setTrackValues(float speed, float distance);

    void setTrackTime(int h, int m, int s);

    void setEmoticons(bool enable);

    void flipVertical(bool enable);

    void setBatteryStatus(float volts, int charge, bool isCharging);

    void setPowerSave();

    void suspendTaskGUI();

    void resumeTaskGUI();

    String getFirmwareVersionCode ();

    void loop();

   private:

    const int pwmFreq = 5000;

    const int pwmResolution = 8;

    const int pwmLedChannelTFT = 0;

#ifdef M5STICKCPLUS
    int backlight[5] = {5, 20, 30, 50, 80};

#else
    int backlight[5] = {10, 30, 60, 120, 220};
#endif

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

    int dw = 0;  // display width

    int dh = 0;  // display height

    uint32_t bufGraphMain[MAX_X];  // Main graph buffer
    
    uint32_t bufGraphMinor[MAX_X];  // Secondary graph buffer

    int state = 0;

    int pressL = 0;

    int pressR = 0;

    uint32_t holdL = 0;

    uint32_t holdR = 0; 

    int wstate = 0;

    int _live_ticks = 0;

    int _rssi = 0;

    int _colorType = 0;

    float _humi = 0.0;

    float _temp = 0.0;

    int _mainValue = 0;

    int _mainUnitId = 0;
    
    int _minorValue = 0;

    String _unit_symbol = "";

    String _unit_name = "";

    int _unit = 0;

    int _average = 0;

    bool _wifi_enable;
    
    bool _pax_enable;

    int _sample_time = 5;

    int _calibration_counter = -2;

    float _speed = 0.0;

    float _km;

    int _hours;

    int _minutes;

    int _seconds;

    bool _wifiOn;

    bool _bleOn;

    bool _blePair;

    bool toggle1s;

    String _info = "";

    bool isNewData;

    bool emoticons;

    float _batteryVolts;

    int _batteryCharge;

    bool _isCharging;

    TaskHandle_t xHandle;

    bool taskGUIrunning;

    int aqicolors[6] = {TFT_GREEN, TFT_YELLOW, TFT_ORANGE, TFT_RED, TFT_PURPLE, TFT_MAROON};

    String aqilabels[6] = {"GOOD", "MODERATE", "UNH SEN G", "UNHEALTY", "VERY UNH", "VERY UNH"};

    void showStatus();

    void showSetup();

    void refreshSetup();
    
    void showInfoWindow();

    void refreshInfoWindow();
    
    void displaySensorAverage(int average);

    void displayMainHeader();
    
    void displayMainUnit(String uName, String uSymbol);

    void displayCenterBig(String msg);
    
    void displayGUIStatusFlags();

    void displayEmoticonLabel(int numsmile, String msg);

    void displayEmoticonColor(uint32_t color, String msg);

    void displayBigEmoticon(String msg);

    void displayBigLabel(int cursor, String msg);

    void resetBuffer(uint32_t * buf);

    double getMultiplicator(uint32_t * buf);

    uint32_t getAQIColor(uint32_t value);
    
    void drawBarGraph();

    void drawDualLineGraph();
    
    void drawLineGraph();

    void drawBluetoothIcon();

    void drawWifiHighIcon();

    void drawWifiMidIcon();

    void drawWifiLowIcon();

    void drawFanIcon();

    void drawDataIcon();

    void drawPreferenceSaveIcon();

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

    void updateCalibrationField();

    void startCalibration();

    void toggleMain();

    void restoreMain();

    void loadLastData();

    void showWindowBike();

    void suspend();

    void setupGUITask();

    GUIUserPreferencesCallbacks* mGUICallBacks = nullptr;

    TFTUtils* getInstance();

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
    virtual void onPowerOff();
    virtual void onUnitSelectionToggle();
    virtual void onUnitSelectionConfirm();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TFTHANDLER)
extern TFTUtils gui;
#endif

#endif
