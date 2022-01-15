#include <Arduino.h>

class BatteryUpdateCallbacks {
public:
    virtual ~BatteryUpdateCallbacks () {};
    virtual void onBatteryUpdate(float voltage, int charge, bool charging);
};

class Battery {
   public:

    int vref = 1100;
    float curv = 0.0;
    bool debug;

    virtual void init(bool debug = false) = 0;
    virtual void update() = 0;
    virtual float getVoltage() = 0;
    virtual int getCharge() = 0;
    virtual bool isCharging() = 0;
    virtual void printValues() = 0;

    void setUpdateCallbacks(BatteryUpdateCallbacks *callbacks) {
        this->callback = callbacks;
    }
    
    void loop() {
        static uint32_t pmLoopTimeStamp = 0;                                // timestamp for sensor loop check data
        if ((millis() - pmLoopTimeStamp > 3000)) {  // sample time for each capture
            pmLoopTimeStamp = millis();
            update();
            if (this->callback != nullptr && isNewVoltage()) {
                pcurv = curv;
                this->callback->onBatteryUpdate(curv, getCharge(), isCharging());
                printValues();
            }
        }
    }

   private:

    BatteryUpdateCallbacks *callback;
    float pcurv = 0.0;

    bool isNewVoltage () {
        return (abs(curv - pcurv) > 0.01);
    }

   protected:

    int calcPercentage(float volts, float max, float min) {
        float percentage = (volts - min) * 100 / (max - min);
        if (percentage > 100) {
            percentage = 100;
        }
        if (percentage < 0) {
            percentage = 0;
        }
        return (int)percentage;
    }

};


