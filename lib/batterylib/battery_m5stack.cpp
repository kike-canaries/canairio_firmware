#include <battery_m5stack.hpp>
#ifdef M5STICKCPLUS
#ifndef DISABLE_BATT

void Battery_M5STACK::init(bool debug) {
    this->debug = debug;
    // M5.Axp.EnableCoulombcounter();  // Enable Coulomb counter.
    setLimits(BATTERY_MIN_V, BATTERY_MAX_V, BATTCHARG_MIN_V, BATTCHARG_MAX_V);
}

float Battery_M5STACK::getVoltage() {
   return curv; 
}

void Battery_M5STACK::update() {
    curv = M5.Power.getBatteryVoltage();
    // vusb = M5.Power.getUsbOutput();
}

bool Battery_M5STACK::isCharging() {
    return M5.Power.getUsbOutput();
}

int Battery_M5STACK::getCharge() {
    if (isCharging()) {
        return calcPercentage(curv, btCharVMax, btCharVMin);
    } else {
        return calcPercentage(curv, btDiscVMax, btDiscVMin);
    }
}

void Battery_M5STACK::printValues() {
    if (!debug) return;
    // Serial.printf("-->[BATT] AXP Temp       \t: %.1fC  \tC: %03d\r\n", M5.Power.GetTempInAXP192(), getCharge());                                          //Get the temperature of AXP192
    Serial.printf("-->[BATT] AXP Bat Volts  \t: %.3fv  \tI: %.3fma\r\n", curv, M5.Power.getBatteryVoltage());  //Output voltage and current of Bat
    // Serial.printf("-->[BATT] AXP USB Volts  \t: %.3fv  \tI: %.3fma\r\n", M5.Axp.GetVBusVoltage(), M5.Axp.GetVBusCurrent());  //Output current and voltage of USB
    Serial.printf("-->[BATT] AXP Bat Level  \t: %.3fv  \tI: %.3fma\r\n", M5.Power.getBatteryLevel(), M5.Power.getBatteryCurrent());
    // Serial.printf("-->[BATT] AXP 5V  Volts  \t: %.3fv  \tI: %.3fma\r\n", M5.Axp.GetVinVoltage(), M5.Axp.GetVinCurrent());
    // Serial.printf("-->[BATT] AXP Bat power  \t: %.3fmw\r\n", M5.Axp.GetBatPower());
}
#endif
#ifdef DISABLE_BATT
void init(bool debug = false) {}
float getVoltage() { return 0.0; }
bool isCharging() { return false; }
int getCharge()   { return 0; }
void printValues() {}
void update() {}
#endif

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_M5STACKBATTERY)
Battery_M5STACK battery;
#endif
#endif
