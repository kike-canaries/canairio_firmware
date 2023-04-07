#include <Arduino.h>
#include <CircularBuffer.h>

#define SLOW_RATE 5000  // time into updates for visualization in ms
#define UPDATES 60      // # of updates before calculate avarage (avarage calc start = SLOW_RATE*UPDATES)
#define FAST_RATE 500   // time into samples for catpure and calculate avarage reference
#define SAMPLES 20      // # of samples on buffer to calculate avarage (sample time = SAMPLES*FAST_RATE)

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
  float lastAverage;
  int isUploading = -1;

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
    if (debug) Serial.printf("-->[BATT] curv:%2.3f pcurv:%2.3f\r\n", curv, pcurv);
    return (abs(curv - pcurv) > 0.015);
  }

 protected:
  float getAverage() {
    float avg = 0.0;
    using index_t = decltype(buffer)::index_t;
    for (index_t i = 0; i < buffer.size(); i++) {
      avg += buffer[i] / (float)buffer.size();
    }
    return avg;
  }

  void capture() {
    buffer.push(curv);
    if (buffer.isFull() && captureStage) {
      if(sampleCount == 0) lastAverage = getAverage();
      Serial.printf("-->[BATT] LastAverage :%2.5f\r\n", lastAverage);
      interval = SLOW_RATE;  // restore slow rate sample
      captureStage = false;
    } 
    if (!captureStage && sampleCount++ > UPDATES) {
      Serial.printf("-->[BATT] reset initial Average\r\n");
      captureStage = true;
      sampleCount = 0;
      interval = FAST_RATE;  // restore fast rate sample for capture
      buffer.clear();
    }
    if (buffer.isFull() && (sampleCount % 10 == 0)) {
      float curAvg = getAverage();
      float diff = abs(lastAverage - curAvg);
      bool uploading = lastAverage >= curAvg || diff < 0.0015;
      Serial.printf("-->[BATT] LA:%2.4f CA:%2.4f DF\t: %2.5f\r\n", lastAverage, curAvg, diff);
      Serial.printf("-->[BATT] Batt is uploading\t: %s\r\n", uploading ? "True" : "False");
      isUploading = (int)uploading;
    }
    // Serial.printf("-->[BATT] capture curv:%2.3f\r\n",curv);
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
