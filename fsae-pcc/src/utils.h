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
#define TIME_STEP_S 0.01F // 10ms time step
#define COMPUTE_ALPHA(CUTOFF_HZ) (1.0F / (1.0F + (1.0F / (2.0F * M_PI * CUTOFF_HZ)) / TIME_STEP_S))
#define LOWPASS_FILTER(NEW, OLD, ALPHA) OLD = ALPHA * NEW + (1.0F - ALPHA) * OLD

#define ACCUMULATOR_VOLTAGE_PIN 14
#define TS_VOLTAGE_PIN 15
#define PCC_RATIO .9

#define PCC_MIN_TIME_MS 6500U // [ms] Minimum time to wait for precharge to complete
#define PCC_MAX_TIME_MS 9000U // [ms] Maximum time to wait for precharge to complete
#define PCC_TARGET_PERCENT 90U // Target precharge percent
#define PCC_SETTLING_TIME 200U // [ms] Time to wait for precharge to settle
#define PCC_MIN_ACC_VOLTAGE 1U // [V] Minimum voltage for shutdown circuit
#define PCC_WAIT_TIME 200U // [ms] Time to wait for stable voltage

#define ACCUM_MIN_VOLTAGE 9

// #define VOLTAGE_DIVIDER_R1 0
// #define VOLTAGE_DIVIDER_R2 1
#define VOLTAGE_DIVIDER_R1 510000 * 5
#define VOLTAGE_DIVIDER_R2 39000
#define VOLTAGE_DIVIDER_RATIO ((VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2) / VOLTAGE_DIVIDER_R2)

#define Rs 12000
#define Rl 100000
#define Rt 6800
#define Ct 1e-8
#define FREQ_TO_VOLTAGE(FREQ) (FREQ * Rt * Ct * Rl * 2.205F / Rs) * VOLTAGE_DIVIDER_RATIO
