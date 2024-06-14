#include <battery_m5stack.hpp>

#ifdef M5STICKCPLUS

void Battery_M5STACK::init(bool debug) {
    this->debug = debug;
    M5.Axp.EnableCoulombcounter();  // Enable Coulomb counter.
    setLimits(BATTERY_MIN_V, BATTERY_MAX_V, BATTCHARG_MIN_V, BATTCHARG_MAX_V);
}

float Battery_M5STACK::getVoltage() {
   return curv; 
}

void Battery_M5STACK::update() {
    curv = M5.Axp.GetBatVoltage();
    vusb = M5.Axp.GetVBusVoltage();
}

bool Battery_M5STACK::isCharging() {
    return M5.axp.GetVBusVoltage() > btCharVMax;
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
    Serial.printf("-->[BATT] AXP Temp       \t: %.1fC  \tC: %03d\r\n", M5.Axp.GetTempInAXP192(), getCharge());                                          //Get the temperature of AXP192
    Serial.printf("-->[BATT] AXP Bat Volts  \t: %.3fv  \tI: %.3fma\r\n", curv, M5.Axp.GetBatCurrent());  //Output voltage and current of Bat
    Serial.printf("-->[BATT] AXP USB Volts  \t: %.3fv  \tI: %.3fma\r\n", M5.Axp.GetVBusVoltage(), M5.Axp.GetVBusCurrent());  //Output current and voltage of USB
    Serial.printf("-->[BATT] AXP 5V  Volts  \t: %.3fv  \tI: %.3fma\r\n", M5.Axp.GetVinVoltage(), M5.Axp.GetVinCurrent());
    Serial.printf("-->[BATT] AXP Bat power  \t: %.3fmw\r\n", M5.Axp.GetBatPower());
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_M5STACKBATTERY)
Battery_M5STACK battery;
#endif

#endif
