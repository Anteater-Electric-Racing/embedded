#include "bms.h"

struct BMS1 bms1;
struct BMS2 bms2;
struct BMS3 bms3;

void BMS_Init() {
    // Initialize BMS data structures with default values
    bms1 = (BMS1){0};
    bms2 = (BMS2){0};
    bms3 = (BMS3){0};
};

BMS1 const* BMS_GetData1() {
    return &bms1;
}

BMS2 const* BMS_GetData2() {
    return &bms2;
}

BMS3 const* BMS_GetData3() {
    return &bms3;
}

void BMS_UpdateData1(BMS1* data) {
    bms1 = *data;
}

void BMS_UpdateData2(BMS2* data) {
    bms2 = *data;
}

void BMS_UpdateData3(BMS3* data) {
    bms3 = *data;
}