#include <wifi.hpp>
#include <MACPool.hpp>
#include <ConfigApp.hpp>

#define WIFI_CHANNEL_MAX 12

void snifferInit();
void snifferStop();
void snifferLoop();
uint16_t getPaxCount();
