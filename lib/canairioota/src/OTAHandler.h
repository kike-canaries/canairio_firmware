#ifndef OTA_Handler_H
#define OTA_Handler_H

class OTAHandler {
    public:
        OTAHandler();
        void setup(const char* ESP_ID, const char* ESP_PASS);
        void loop();
        void setBaud(int baud);
    private:
        const char* _ESP_ID;
        const char* _ESP_PASS;
        int _baud;
};
#endif
