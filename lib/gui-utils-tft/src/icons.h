#if defined(__AVR__)
    #include <avr/pgmspace.h>
#elif defined(__PIC32MX__)
    #define PROGMEM
#elif defined(__arm__)
    #define PROGMEM
#endif

// 'arrows', 12x16px
const uint8_t iconArrows [] PROGMEM = {
	0xff, 0xf0, 0xff, 0xf0, 0xee, 0x30, 0xc6, 0x30, 0xc6, 0x30, 0x82, 0x30, 0x82, 0x30, 0xc6, 0x30, 
	0xc6, 0x30, 0xc4, 0x10, 0xc4, 0x10, 0xc6, 0x30, 0xc6, 0x30, 0xc7, 0x70, 0xff, 0xf0, 0xff, 0xf0
};

// 'icon_fan_state0', 12x16px
const unsigned char iconFanState0 [] PROGMEM = {
	0xff, 0xf0, 0xff, 0xf0, 0xfe, 0xf0, 0xfe, 0xf0, 0xfe, 0xf0, 0xfe, 0xf0, 0x80, 0xf0, 0xf2, 0xf0, 
	0xf4, 0xf0, 0xf0, 0x10, 0xf7, 0xf0, 0xf7, 0xf0, 0xf7, 0xf0, 0xf7, 0xf0, 0xff, 0xf0, 0xff, 0xf0
};

// 'icon_fan_state1', 12x16px
const unsigned char iconFanState1 [] PROGMEM = {
	0xff, 0xf0, 0xff, 0xf0, 0xf7, 0xf0, 0xf7, 0xf0, 0xf7, 0xf0, 0xf7, 0xf0, 0xf0, 0x10, 0xf2, 0xf0, 
	0xf4, 0xf0, 0x80, 0xf0, 0xfe, 0xf0, 0xfe, 0xf0, 0xfe, 0xf0, 0xfe, 0xf0, 0xff, 0xf0, 0xff, 0xf0
};

// 'icon_bluetooth_pair', 12x16px
const unsigned char iconBluetoothPaired [] PROGMEM = {
	0xff, 0xf0, 0xff, 0xf0, 0xf8, 0xf0, 0xb8, 0x70, 0x9b, 0x30, 0xcb, 0x30, 0xe2, 0x70, 0xb0, 0xd0, 
	0xb0, 0xd0, 0xe2, 0x70, 0xcb, 0x30, 0x9b, 0x30, 0xb8, 0x70, 0xf8, 0xf0, 0xff, 0xf0, 0xff, 0xf0
};

// 'icon_wifi_low', 12x16px
const unsigned char iconWifiLow [] PROGMEM = {
	0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 
	0xff, 0xf0, 0xf0, 0xf0, 0xe0, 0x70, 0xef, 0x70, 0xf9, 0xf0, 0xf9, 0xf0, 0xff, 0xf0, 0xff, 0xf0
};

// 'icon_wifi_mid', 12x16px
const unsigned char iconWifiMid [] PROGMEM = {
	0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xe0, 0x70, 0xc0, 0x30, 
	0xdf, 0xb0, 0xf0, 0xf0, 0xe0, 0x70, 0xef, 0x70, 0xf9, 0xf0, 0xf9, 0xf0, 0xff, 0xf0, 0xff, 0xf0
};

// 'icon_wifi_high', 12x16px
const unsigned char iconWifiHigh [] PROGMEM = {
	0xff, 0xf0, 0xff, 0xf0, 0xff, 0xf0, 0xc0, 0x30, 0x80, 0x10, 0xbf, 0xd0, 0xe0, 0x70, 0xc0, 0x30, 
	0xdf, 0xb0, 0xf0, 0xf0, 0xe0, 0x70, 0xef, 0x70, 0xf9, 0xf0, 0xf9, 0xf0, 0xff, 0xf0, 0xff, 0xf0
};