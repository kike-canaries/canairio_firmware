// TTGO_TDISPLAY HAL DEFINTIONS
#ifdef TTGO_TDISPLAY

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN 0x10
#endif

#define ADC_EN 14
#define ADC_PIN 34
#define BUTTON_R 35
#define BUTTON_L 0
#endif

#ifdef M5STICKCPLUS
#define BUTTON_R 37
#define BUTTON_L 39
#endif

#ifdef ESP32S3
#define BUTTON_R 0
#define BUTTON_L 0
#define TFT_BL 45
#endif

#ifndef M5STICKCPLUS
  #ifndef TTGO_TDISPLAY
  // put here for other boards like OLED
  #endif
#endif

