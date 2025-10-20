#include "power_limit.h"
#include "utils/utils.h"

float PowerLimit_Update(float soc, float voltage, float current, float driverTorqueCmd){
    float power = voltage * current;
    if (power > TORQUE_THRESHOLD){ // define constant in utils, currently dummy value
        return TORQUE_THRESHOLD;
    }
    else if (soc < SOC_THRESHOLD){
        return 0;
    }
}
