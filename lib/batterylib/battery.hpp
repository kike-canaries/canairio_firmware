#include <Arduino.h>
#include <CircularBuffer.h>

#define SLOW_RATE 5000  // time into updates for visualization in ms
#define FAST_RATE 500   // time into samples for catpure initial buffer
#define SAMPLES 60      // # of samples on buffer (sample time = SAMPLES*FAST_RATE)

class BatteryUpdateCallbacks {
 public:
  virtual ~BatteryUpdateCallbacks(){};
  virtual void onBatteryUpdate(float voltage, int charge, bool charging);
};

class Battery {
 public:
  float curv = 0.0;
  bool debug;

  bool captureStage = true;
  CircularBuffer<float, SAMPLES> buffer;
  int isDischarging = -1;

  virtual void init(bool debug = false) = 0;
  virtual void update() = 0;
  virtual float getVoltage() = 0;
  virtual int getCharge() = 0;
  virtual bool isCharging() = 0;
  virtual void printValues() = 0;

  float getAverage() {
    float avg = 0.0;
    using index_t = decltype(buffer)::index_t;
    for (index_t i = 0; i < buffer.size(); i++) {
      avg += buffer[i] / (float)buffer.size();
    }
    return avg;
  }

  float getSlope() {
    if (!buffer.isFull()) return 0.0;
    float sumX = 0, sumY = 0, sumXY = 0, sumXsq = 0;
    using index_t = decltype(buffer)::index_t;
    for (index_t i = 0; i < buffer.size(); i++) {
        sumX += i;
        sumY += buffer[i];
        sumXY += i * buffer[i];
        sumXsq += i * i;
    }
    float m = ((SAMPLES * sumXY) - (sumX * sumY)) / ((SAMPLES * sumXsq) - (sumX * sumX));
    return m;
  }

  void setUpdateCallbacks(BatteryUpdateCallbacks *callbacks) {
    this->callback = callbacks;
  }

  void loop() {
    static uint32_t pmLoopTimeStamp = 0;            // timestamp for sensor loop check data
    if ((millis() - pmLoopTimeStamp > interval)) {  // sample time for each capture
      pmLoopTimeStamp = millis();
      update();
      notify();
      capture();
    }
  }

 private:
  BatteryUpdateCallbacks *callback;
  float pcurv = 0.0;
  int interval = FAST_RATE;
  int sampleCount = 0;

  bool isNewVoltage() {
    // if (debug) Serial.printf("-->[BATT] curv:%2.3f pcurv:%2.3f\r\n", curv, pcurv);
    return (abs(curv - pcurv) > 0.015);
  }

 protected:

  void capture() {
    buffer.push(curv);
    if (buffer.isFull() && captureStage) {
      interval = SLOW_RATE;  // restore slow rate sample
      captureStage = false;
      if (debug) Serial.println("-->[BATT] first capture ready.");
    } 
    if (buffer.isFull() && (sampleCount % 2 == 0)) {
      float average = getAverage();
      float slope = getSlope();
      bool discharging = slope <= 0;
      if (debug) Serial.printf("-->[BATT] avarage: %2.4f slope\t: %2.6f\r\n", average, slope);
      if (debug) Serial.printf("-->[BATT] is discharging  \t: %s\r\n", discharging ? "True" : "False");
      isDischarging = (int) discharging;
    }
    if (sampleCount++ > 100) sampleCount = 0;
    // if (debug && !buffer.isFull() )Serial.printf("-->[BATT] push to buffer volt\t: %2.4f\r\n",curv);
  }

  void notify() {
    if (this->callback != nullptr && isNewVoltage()) {
      pcurv = curv;
      this->callback->onBatteryUpdate(curv, getCharge(), isCharging());
      printValues();
    }
  }

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
