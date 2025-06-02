#ifndef PRECHARGE_H
#define PRECHARGE_H

#include "gpio.h"

#define LOW 0
#define HIGH 1

// Lowpass filter
#define TIME_STEP 0.01F // 10ms time step
#define COMPUTE_ALPHA(CUTOFF_HZ) \
    (1.0F / (1.0F + (1.0F / (2.0F * M_PI * CUTOFF_HZ)) / TIME_STEP))
#define LOWPASS_FILTER(NEW, OLD, ALPHA) \
    OLD = ALPHA * NEW + (1.0F - ALPHA) * OLD

void prechargeInit();
void standby();
void precharge();
void running();
void errorState();
void updateStatusLeds();
void statusLEDsOff();
void prechargeTask(void *pvParameters);

#endif
