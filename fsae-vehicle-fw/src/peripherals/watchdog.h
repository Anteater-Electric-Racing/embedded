// Anteater Electric Racing, 2025

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "Watchdog_t4.h"
#include <Arduino.h>

#define WDT_WINDOW_SEC 0.5
#define WDT_TIMEOUT_SEC 1.0
#define WDT_TRIGGER_SEC 0.8
/*
static struct { // delete this later
    volatile unsigned long brake;
    volatile unsigned long suspension;
    volatile unsigned long pedal;
    volatile unsigned long imu;
    volatile unsigned long gps;
} sensorLastUpdate = {0, 0, 0, 0, 0}; */

bool checkSensors();

void Watchdog_Init();

void Watchdog_Pet();

void myCallBack();

#endif
