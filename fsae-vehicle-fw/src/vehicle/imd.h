#pragma once

#include "ifl100-36.h"

#include "arduino_freertos.h"

#include "peripherals/can.h"

#include "vehicle/telemetry.h"

#include "utils/utils.h"

#include "pcc_receive.h"

uint64_t PackIMDData(uint8_t index, uint16_t value);
void IMD_UpdateThreshold(uint16_t threshold_kOhm);
void IMD_InitializeConfiguration();
