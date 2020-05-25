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

  unsigned int inthumi = 0;
  unsigned int inttemp = 0;

  unsigned int cursor = 0;

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

  void showProgress(unsigned int progress, unsigned int total);

  void welcomeAddMessage(String msg);

  void displayCenterBig(String msg);

  void displayBottomLine(String msg);

  void displayEndLine(String msg);

  void displayEmoticonLabel(int numsmile, String msg);

  void displayBigEmoticon(String msg);

  void displayBigLabel(int cursor, String msg);

  void displaySensorAverage(int average);

  void displaySensorData(int pm25, int pm10, int chargeLevel, float humi, float temp);

  void displayStatus(bool wifiOn, bool bleOn, bool blePair, bool dataOn);

  void displayLiveIcon();
  
  void displayPrefSaveIcon(bool enable);

  //void displaySensorData(int pm25, int pm10, float humi, float temp);

  void updateError(unsigned int error);

  void pageStart();

  void pageEnd();

};

#endif
