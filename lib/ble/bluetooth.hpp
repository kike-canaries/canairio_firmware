#ifndef DISABLE_BLE

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "GUILib.hpp"
#include "Sensors.hpp"
#include "sniffer.hpp"
#include "Batterylib.hpp"

#define SERVICE_UUID        "c8d1d262-861f-4082-947e-f383a259aaf3"
#define CHARAC_DATA_UUID    "b0f332a8-a5aa-4f3f-bb43-f99e7791ae01"
#define CHARAC_CONFIG_UUID  "b0f332a8-a5aa-4f3f-bb43-f99e7791ae02"
#define CHARAC_STATUS_UUID  "b0f332a8-a5aa-4f3f-bb43-f99e7791ae03"

void bleLoop();
void bleServerInit();
bool bleIsConnected();
void bleServerConfigRefresh();
#endif