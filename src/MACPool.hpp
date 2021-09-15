#ifndef MACPool_hpp
#define MACPool_hpp

#include <Arduino.h>

class MACPool {
   private:
    String mac;
    int signal;
    unsigned long time;
    bool newMAC;

   public:
    MACPool(String, int, unsigned long, bool);
    String getMAC();
    int getSignal();
    unsigned long getTime();
    void updateTime(unsigned long);
    bool getNewMAC();
    void updateNewMAC(bool);
};

#endif