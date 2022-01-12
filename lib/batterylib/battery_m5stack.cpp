#include <battery_m5stack.hpp>

#ifdef M5STICKCPLUS

void Battery_M5STACK::init() {
}

float Battery_M5STACK::getVoltage() {
    if (isCharging())
        curv = M5.Axp.GetBatVoltage();
    else
        curv = M5.Axp.GetBatVoltage();
    return curv;
}

bool Battery_M5STACK::isCharging() {
    return M5.Axp.GetVBusCurrent() > 0;
}

void Battery_M5STACK::printValues() {
    Serial.printf("-->[BATT] AXP Temp\t: %.1fC\n", M5.Axp.GetTempInAXP192());                                          //Get the temperature of AXP192
    Serial.printf("-->[BATT] AXP Bat Volts\t: %.3fv  \tI: %.3fma\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());  //Output voltage and current of Bat
    Serial.printf("-->[BATT] AXP USB Volts\t: %.3fv  \tI: %.3fma\n", M5.Axp.GetVBusVoltage(), M5.Axp.GetVBusCurrent());  //Output current and voltage of USB
    Serial.printf("-->[BATT] AXP 5V  Volts\t: %.3fv  \tI: %.3fma\n", M5.Axp.GetVinVoltage(), M5.Axp.GetVinCurrent());
    Serial.printf("-->[BATT] AXP Bat power\t: %.3fmw\n", M5.Axp.GetBatPower());
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_M5STACKBATTERY)
Battery_M5STACK battery;
#endif

#endif