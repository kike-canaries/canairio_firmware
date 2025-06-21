#ifndef SNIFFER_H
#define SNIFFER_H
#ifndef DISABLE_SNIFFER

#include <wifi.hpp>
#include <MACPool.hpp>
#include "ConfigApp.hpp"

using namespace std;
#include <vector>

#define WIFI_CHANNEL_MAX 13

#endif
void snifferInit();
void snifferStop();
void snifferLoop();
unsigned int getPaxCount();
#endif
