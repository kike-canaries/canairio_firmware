/***
 * TESTING U8G2Lib
 * @hpsaturn 
 ***/

#include <Arduino.h>
#include <U8g2lib.h>

// Firmware version from git rev-list command
String VERSION_CODE = "rev";
#ifdef SRC_REV
int VCODE = SRC_REV;
#else
int VCODE = 0;
#endif

unsigned int mcount, ecount = 0;

#ifdef WEMOSOLED // display via i2c for WeMOS OLED board
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);
#elif HELTEC // display via i2c for Heltec board
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 15, 4, 16);
#else       // display via i2c for D1MINI board
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE,U8X8_PIN_NONE,U8X8_PIN_NONE);
#endif

#define bitmap_name_height 8
#define bitmap_name_width 8

static const unsigned char PROGMEM test_margin[] = {
 B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
 B01111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111110,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000010,
 B01111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111110,
 B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000
};

static const unsigned char PROGMEM test_margin_64x48_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x7f, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xfe, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const unsigned char PROGMEM ic_bluetooth_on[] = {
 B00000000,
 B00011100,
 B01010110,
 B00111100,
 B00111000,
 B00111100,
 B01010110,
 B00011100,
};

static const unsigned char PROGMEM ic_bluetooth_off[] = {
 B00000000,
 B00011100,
 B01010110,
 B00111100,
 B10111001,
 B00111100,
 B01010110,
 B00011100,
};

static const unsigned char PROGMEM ic_wifi_on[] = {
 B00000000,
 B00011000,
 B00111100,
 B01000010,
 B10011001,
 B00100100,
 B01000010,
 B00011000,
};

static const unsigned char PROGMEM ic_data_1[] = {
 B00000000,
 B00111000,
 B01101100,
 B11000110,
 B00000000,
 B11000110,
 B01101100,
 B00111000,
};

static const unsigned char PROGMEM ic_data_2[] = {
 B00100100,
 B01110100,
 B01110100,
 B00100100,
 B00100100,
 B00101110,
 B00101110,
 B00100100,
};

static const unsigned char PROGMEM ic_data_3[] = {
 B00000000,
 B01100110,
 B11110110,
 B01100110,
 B01100110,
 B01101111,
 B01100110,
 B00000000,
};

bool toggle;

/******************************************************************************
*   D I S P L A Y  M E T H O D S
******************************************************************************/
void displayInit(){
  Serial.println("-->[OLED] setup display..");
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setContrast(255);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  u8g2.setFontMode(0);
  Serial.println("-->[OLED] display ready.");
}

void showWelcome(){
  u8g2.firstPage();  // only for first screen
#ifdef D1MINI
  u8g2.drawStr(0, 0, "CanAirIO");
  String version = "("+String(VERSION_CODE+VCODE)+")";
  u8g2.drawStr(0, 11,version.c_str());
  u8g2.drawLine(0, 22, 128, 22);
#else
  String version = "CanAirIO ("+String(VERSION_CODE+VCODE)+")";
  u8g2.drawStr(0, 0,version.c_str());
  u8g2.drawLine(0, 11, 128, 11);
#endif
  // only for first screen
  Serial.println("-->[OLED] welcome screen ready\n");
  u8g2.nextPage();
}

void displayCenterBig(String msg){
#ifdef D1MINI
  u8g2.setCursor(0,0);
  u8g2.setFont(u8g2_font_inb24_mn);
#else
  u8g2.setCursor(73,40);
  u8g2.setFont(u8g2_font_freedoomr25_mn);
#endif
  u8g2.print(msg.c_str());
}

void displayBottomLine(String msg){
  u8g2.setFont(u8g2_font_4x6_tf);
#ifdef D1MINI
  u8g2.setCursor(0, 27);
  u8g2.print(msg.c_str());
#else
  u8g2.setCursor(0, 16);
  u8g2.print(msg.c_str());
#endif
}

void displayEndLine(String msg){
  u8g2.setFont(u8g2_font_4x6_tf);
#ifdef D1MINI
  u8g2.setCursor(0, 33);
  u8g2.print(msg.c_str());
#else
  u8g2.setCursor(0, 16);
  u8g2.print(msg.c_str());
#endif
}

void displaySensorError(){
  char output[22];
  if(ecount>999)ecount=0;
  sprintf(output,"%04d E:%03d",mcount,ecount++);
  displayEndLine(String(output));
}

void displaySensorAvarage(int avarage){
  char output[4];
  sprintf(output, "%03d", avarage);
  displayCenterBig(output);
}

void displaySensorData(int pm25, int pm10){
  char output[22];
#ifdef D1MINI
  sprintf(output, "%03d %03d %02d %05d", pm25, pm10, ecount, mcount);
#else
  sprintf(output, "%04d P25:%03d P10:%03d", mcount, pm25, pm10);
#endif
  Serial.println(" --> " + String(output) + " E:" + String(ecount));
  displayBottomLine(String(output));
}


void setup(void) {
  Serial.begin(115200);
  Serial.println("\n== INIT SETUP ==\n");
  Serial.println("-->[SETUP] console ready");
  displayInit();
  showWelcome();
  delay(1000);
}

void loop(void) {
  u8g2.firstPage();
  do {
    if (ecount == 99) ecount = 0;
    if (mcount == 999) mcount = 0;
    displaySensorAvarage(mcount); // it was calculated on bleLoop()
    displaySensorData(ecount++,mcount++);
    displaySensorError();
    if(ecount % 100 == 0)toggle = !toggle;
    if(toggle) u8g2.drawBitmap(54,40,1,8,ic_bluetooth_on);
    else u8g2.drawBitmap(54,40,1,8,ic_bluetooth_off);
    u8g2.drawBitmap(44,40,1,8,ic_wifi_on);
    // u8g2.drawBitmap(20,40,1,8,ic_data_1);
    // u8g2.drawBitmap(30,40,1,8,ic_data_2);
    u8g2.drawBitmap(34,40,1,8,ic_data_3);
    //u8g2.drawBitmap(0,0,8,48,test_margin);
    // u8g2.drawBitmap(0,0,8,48,test_margin_64x48_bits);
  } while (u8g2.nextPage());
  delay(10);
}

