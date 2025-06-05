// Anteater Electric Racing, 2025

#pragma once

// monitorShutdownCircuitTask defines
#define ACCUMULATOR_VOLTAGE_PIN 14
#define TS_VOLTAGE_PIN 15
#define PCC_RATIO .9


#define PCC_MIN_TIME_MS 500U // [ms] Minimum time to wait for precharge to complete
#define PCC_MAX_TIME_MS 3000U // [ms] Maximum time to wait for precharge to complete
#define PCC_TARGET_PERCENT 90U // Target precharge percent
#define PCC_SETTLING_TIME 200U // [ms] Time to wait for precharge to settle
#define PCC_MIN_ACC_VOLTAGE 100U // [V] Minimum voltage for shutdown circuit
#define PCC_WAIT_TIME 200U // [ms] Time to wait for stable voltage

enum PrechargeState {
    STATE_STANDBY,
    STATE_PRECHARGE,
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

float getFrequency(int pin);
float getVoltage(int pin);

void prechargeInit();
void standby();
void precharge();
void running();
void errorState();
void updateStatusLeds();
void statusLEDsOff();

float getTSVoltage();
float getAccumulatorVoltage();
PrechargeState getPrechargeState();
int getPrechargeError();