#ifndef PRECHARGE_H
#define PRECHARGE_H

#include "gpio.h"

#define LOW 0
#define HIGH 1

void prechargeInit();
void standby();
void precharge();
void running();
void errorState();
void updateStatusLeds();
void statusLEDsOff();
void prechargeTask(void *pvParameters);

#endif
