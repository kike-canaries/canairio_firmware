#include "sniffer.h"

/******************************************************************************
*   P A X   C O U N T E R  
******************************************************************************/

bool sniffer_start;
uint_fast16_t pax_count, last_pax_count;
unsigned int channel;

vector<MACPool> listOfMAC;

const wifi_promiscuous_filter_t filt={
    .filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT|WIFI_PROMIS_FILTER_MASK_DATA
};

typedef struct {
  uint8_t mac[6];
} __attribute__((packed)) MacAddr;

typedef struct {
  int16_t fctl;
  int16_t duration;
  MacAddr da;
  MacAddr sa;
  MacAddr bssid;
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;

void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
  
    wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf;
    WifiMgmtHdr *wh = (WifiMgmtHdr*)p->payload;
    int signal = p->rx_ctrl.rssi;

    MacAddr mac_add = (MacAddr)wh->sa;
    String sourceMac;
    for (int i = 0; i < sizeof(mac_add.mac); i++) {
        String macDigit = String(mac_add.mac[i], HEX);
        if (macDigit.length() == 1) {
            macDigit = "0" + macDigit;
        }
        
        sourceMac += macDigit;
        if (i != sizeof(mac_add.mac) - 1) {
          sourceMac += ":";
        }
    }

    sourceMac.toUpperCase();

    if (signal > -70) { // signal level threshold

      // Prevent duplicates
      for (int i = 0; i < listOfMAC.size(); i++) {
          if (sourceMac == listOfMAC[i].getMAC()) {
              listOfMAC[i].updateTime(millis()); // update the last time MAC found
              listOfMAC[i].updateNewMAC(false);
              return;
          }
      }

      // new MAC
      listOfMAC.push_back(MACPool(sourceMac,signal,millis(),true));
      log_d("[WIFI] %s",listOfMAC[listOfMAC.size()-1].getMAC().c_str());

      // purge outdated MACs
      for (auto it = listOfMAC.begin(); it != listOfMAC.end(); ) {
          if (millis()-it->getTime() > 60000) { // remove if older than 1min
              it = listOfMAC.erase(it);
          } else {
              ++it;
          }
      }

      pax_count = listOfMAC.size();
      if (pax_count != last_pax_count) {
          if(devmode) Serial.printf("-->[WIFI] new PAX count found\t: %d\r\n",pax_count);
          last_pax_count = pax_count;
      }
      delay(10);
    }
}

void wifiScanChannels() {
    channel = (channel % WIFI_CHANNEL_MAX) + 1;
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

void snifferInit() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    //set WiFi in promiscuous mode
    //esp_wifi_set_mode(WIFI_MODE_STA);            // Promiscuous works only with station mode
    esp_wifi_set_mode(WIFI_MODE_NULL);
    // power save options
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
    // esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    esp_wifi_start();
    // esp_wifi_set_max_tx_power(-4);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(&filt);
    esp_wifi_set_promiscuous_rx_cb(&sniffer);      // Set up promiscuous callback
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    wifiScanChannels();    
    sniffer_start = true;
    Serial.println("-->[WIFI] started PAX counter\t: sniffer ON ;)");
}

void snifferStop () {
    // TODO: cleaer the vector of MACs??
    Serial.println("-->[WIFI] stoping PAX counter sniffer..");
    esp_wifi_set_promiscuous(false);
    esp_wifi_stop();
    delay(100);
    pax_count = 0;
    sniffer_start = false;
}

void snifferLoop() {
    static uint32_t snifferTimeStamp = 0;                                  // timestamp for sensor loop check data
    if ((millis() - snifferTimeStamp > stime / 5 * (uint32_t)500)) {  // sample time for each capture
        snifferTimeStamp = millis();
        if (!isWifiEnable() && isPaxEnable() && !sniffer_start) snifferInit();
        else if (!isWifiEnable() && isPaxEnable() && sniffer_start) wifiScanChannels();
        else if (!isWifiEnable() && !isPaxEnable() && sniffer_start) snifferStop();
    }
}

uint16_t getPaxCount(){
    return pax_count;
}