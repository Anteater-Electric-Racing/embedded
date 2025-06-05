// Anteater Electric Racing, 2025

#pragma once

#include "Arduino.h"
#include <stdint.h>

#define LOW 0
#define HIGH 1

// Relays
#define SHUTDOWN_CTRL_PIN 11

// Frequency measurements (from Voltage-to-Frequency converters)
#define FREQ_ACCU_PIN 14
#define FREQ_TS_PIN 15

// Lowpass filter
#define TIME_STEP 0.01F // 10ms time step
#define COMPUTE_ALPHA(CUTOFF_HZ) (1.0F / (1.0F + (1.0F / (2.0F * M_PI * CUTOFF_HZ)) / TIME_STEP))
#define LOWPASS_FILTER(NEW, OLD, ALPHA) OLD = ALPHA * NEW + (1.0F - ALPHA) * OLD

#define Rs 12000
#define Rl 100000
#define Rt 6800
#define Ct 1e-8
#define FREQ_TO_VOLTAGE(FREQ) (FREQ * Rt * Ct * Rl * 2.25 / Rs)