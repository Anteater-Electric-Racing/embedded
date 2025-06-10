// Anteater Electric Racing, 2025

#pragma once

enum PrechargeState {
    STATE_STANDBY,
    STATE_PRECHARGE,
    STATE_DISCHARGE,
    STATE_ONLINE,
    STATE_ERROR,
    STATE_UNDEFINED
};

enum {
    ERR_NONE = 0b00000000,
    ERR_PRECHARGE_TOO_FAST = 0b00000001,
    ERR_PRECHARGE_TOO_SLOW = 0b00000010,


    ERR_STATE_UNDEFINED = 0b10000000,
};

void prechargeInit();

float getTSVoltage();
float getAccumulatorVoltage();
PrechargeState getPrechargeState();
int getPrechargeError();
