// Anteater Electric Racing, 2025

#define BSE_LOWER_THRESHOLD 0.5
#define BSE_UPPER_THRESHOLD 4.5
#define BSE_IMPLAUSABILITY_THRESHOLD 0.1

#include <cmath>

#include "bse.h"

#include "vehicle/faults.h"

BSE::BSE() {
    _BSEdata.bseReading1 = 0;
    _BSEdata.bseReading2 = 0;
}

BSE::~BSE() {}

void BSE::checkAndHandleBSEFault() {
    if (_BSEdata.bseReading1 < BSE_LOWER_THRESHOLD ||
        _BSEdata.bseReading1 > BSE_UPPER_THRESHOLD) {
        Faults_SetFault(FAULT_BSE);
    } else {
        Faults_ClearFault(FAULT_BSE);
    }
}

void BSE::verifySensorAgreement() {
    // should notify telemetry if brakes have a disagreement
    if (abs(_BSEdata.bseReading1 - _BSEdata.bseReading2) >
        BSE_IMPLAUSABILITY_THRESHOLD) {
        // set fault code
    } else {
        // clear fault code
    }
}

void BSE::BSE_UpdateData(BSEData *data) {
    _BSEdata.bseReading1 = data->bseReading1;
    _BSEdata.bseReading2 = data->bseReading2;

    checkAndHandleBSEFault();
    verifySensorAgreement();
}

float BSE::BSE_GetBSEReading() {
    // regen braking
    return (_BSEdata.bseReading1 + _BSEdata.bseReading2) / 2;
}
