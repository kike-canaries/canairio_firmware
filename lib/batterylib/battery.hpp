#include <Arduino.h>

#define BATTERY_MIN_V 3.2
#define BATTERY_MAX_V 4.1
#define BATTCHARG_MIN_V 4.65
#define BATTCHARG_MAX_V 4.8

class Battery {
   public:

    int vref = 1100;
    float curv = 0;
    bool debug;

    virtual void init(bool debug = false) = 0;
    virtual void update() = 0;
    virtual float getVoltage() = 0;
    virtual bool isCharging() = 0;
    virtual void printValues() = 0;

    int getCharge() {
        if (isCharging()) {
            return calcPercentage(curv, BATTCHARG_MAX_V, BATTCHARG_MIN_V);
        } else {
            return calcPercentage(curv, BATTERY_MAX_V, BATTERY_MIN_V);
        }
    }

    void loop() {
        static uint32_t pmLoopTimeStamp = 0;                                // timestamp for sensor loop check data
        if ((millis() - pmLoopTimeStamp > 5000)) {  // sample time for each capture
            pmLoopTimeStamp = millis();
            update();
            printValues();
        }
    }

   private:
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
