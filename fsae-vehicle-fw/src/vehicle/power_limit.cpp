#include "power_limit.h"
#include "utils/utils.h"

float PowerLimit_Update(float soc, float voltage, float current, float driverTorqueCmd){
    float power = voltage * current;

    // 1. Compute torqueLimit using Linear SOC Derating
    float torqueLimit;
    if (soc >= SOC_START){
        torqueLimit = MOTOR_MAX_TORQUE;
    }

    else if (soc > SOC_END){
        torqueLimit = (soc - SOC_END)/(SOC_START-SOC_END) * MOTOR_MAX_TORQUE;
    }

    else{
        torqueLimit = 0.0f;
    }

    // 2. Scale torque if electrical power exceeds limit
    if (power > TORQUE_THRESHOLD){ // define constant in utils, currently dummy value
        driverTorqueCmd *= TORQUE_THRESHOLD / power;
    }

    // 3. Force driverTorqueCmd to to stay under torque limit
    if (driverTorqueCmd > torqueLimit){
        driverTorqueCmd = torqueLimit;
    }

    else if (driverTorqueCmd < -torqueLimit){
        driverTorqueCmd = -torqueLimit;
    }

    return driverTorqueCmd;
}
