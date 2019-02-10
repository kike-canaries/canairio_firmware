#ifndef GUIUtils_hpp
#define GUIUtils_hpp

#include <U8g2lib.h>

class GUIUtils
{
private:

public:

  GUIUtils(void) {};

  unsigned int mcount, ecount = 0;

  U8G2 u8g2;

  // Firmware version from git rev-list command
  String VERSION_CODE = "rev";
#ifdef SRC_REV
  int VCODE = SRC_REV;
#else
  int VCODE = 0;
#endif

  void displayInit(U8G2 &u8g2);

  void showWelcome();

  void displayCenterBig(String msg);

  void displayBottomLine(String msg);

  void displayEndLine(String msg);

  void displayStatus(bool wifiOn, bool bleOn, bool blePair, bool dataOn);

  void displaySensorAvarage(int avarage);

  void displaySensorData(int pm25, int pm10);

  void updateError();

  void pageStart();

  void pageEnd();

};

#endif
