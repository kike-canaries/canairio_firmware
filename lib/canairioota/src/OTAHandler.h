#ifndef OTA_Handler_H
#define OTA_Handler_H

#ifdef ENABLE_OTA
#include <ArduinoOTA.h>
#endif

#include <ESPmDNS.h>
#include <WiFiUdp.h>

#include <esp_system.h>
#include <esp32fota.h>
#include <esp_task_wdt.h>

#define FOTA_CHECK_INTERVAL 120   // seconds

typedef void (*voidMessageCbFn)(const char *msg);

class OTAHandlerCallbacks;
class OTAHandler {
    public:
        OTAHandler();
        void setup(const char* ESP_ID, const char* ESP_PASS);
        void setCallbacks(OTAHandlerCallbacks* pCallBacks);
        void setOnUpdateMessageCb(voidMessageCbFn cb);
        void loop();
        void checkRemoteOTA(bool notify = true);
        void setBaud(int baud);
        bool isConfigured();
        OTAHandler* getInstance();
    private:
        OTAHandlerCallbacks* m_pOTAHandlerCallbacks = nullptr;
        voidMessageCbFn _onUpdateMsgCb = nullptr;
        const char* _ESP_ID;
        const char* _ESP_PASS;
        int _baud;
        bool configured = false;
        void remoteOTAcheckloop();
};

class OTAHandlerCallbacks {
 public:
  virtual ~OTAHandlerCallbacks(){};
  virtual void onStart() = 0;
  virtual void onProgress(unsigned int progress, unsigned int total) = 0;
  virtual void onEnd() = 0;
  virtual void onError() = 0;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OTAHANDLER)
extern OTAHandler ota;
#endif

#endif
