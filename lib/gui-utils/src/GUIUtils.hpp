#ifndef GUIUtils_hpp
#define GUIUtils_hpp

#include <U8g2lib.h>

class GUIUtils
{
private:

  bool toggleLive;

public:

  GUIUtils(void) {};

  unsigned int mcount, ecode = 0;

  int lastDrawedLine = 0;

  unsigned int inthumi, inttemp = 0;

  U8G2 u8g2;

  // Firmware version from git rev-list command
  String VERSION_CODE = "r";
#ifdef SRC_REV
  int VCODE = SRC_REV;
#else
  int VCODE = 0;
#endif

  void displayInit(U8G2 &u8g2);

  void showWelcome();

  void welcomeAddMessage(String msg);

  void showProgress(unsigned int progress, unsigned int total);

  void displayCenterBig(String msg);

  void displayBottomLine(String msg);

  void displayEndLine(String msg);

  void displayStatus(bool wifiOn, bool bleOn, bool blePair, bool dataOn);

  void displayLiveIcon();
  
  void displayPrefSaveIcon(bool enable);

  void displaySensorAvarage(int avarage);

  void displaySensorData(int pm25, int pm10, float humi, float temp);

  void displaySensorData(int pm25, int pm10, int chargeLevel, float humi, float temp);

  void updateError(unsigned int error);

  void pageStart();

  void pageEnd();

};

#endif
