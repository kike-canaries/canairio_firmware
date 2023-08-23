#pragma once
#ifndef _LORAWAN_H
#define _LORAWAN_H

#include <Arduino.h>
#include <lmic.h>
#include "lora_credentials.h"
#include <pb.h>
#include <pb_common.h>
#include <pb_encode.h>

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = TTN_APPEUI;

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = TTN_DEVEUI;

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = TTN_APPKEY;

static uint8_t LORA_DATA[] = "Only test";
//static uint8_t LORA_DATA[21];
static osjob_t sendjob;

// LMIC State save for reboot
extern RTC_DATA_ATTR lmic_t RTC_LMIC;

void os_getArtEui(u1_t *buf);
void os_getDevEui(u1_t *buf);
void os_getDevKey(u1_t *buf);

void LoRaWANSetup(void);
void LoraWANDo_send(osjob_t *j);
void LoraWANDo(void);
void LoraWANGetData(void);
void LoraWANSaveLMICToRTC(int deepsleep_sec);
void LoraWANLoadLMICFromRTC(void);
void LoraWANPrintVersion(void);
void LoraWANPrintLMICOpmode(void);
void LoraWANDebug(lmic_t lmic_to_check);

#endif