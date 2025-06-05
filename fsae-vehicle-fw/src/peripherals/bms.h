
typedef struct __attribute__((packed)) {
    uint64_t PackCurrent : 16;
    uint64_t InstVoltage : 16;
    uint64_t SOC: 8;
    uint64_t RelayStatus : 16;
    uint64_t Checksum : 8;
} BMS1;

typedef struct __attribute__((packed)) {
    uint64_t DCL : 32;
    uint64_t HighTemp : 8;
    uint64_t LowTemp: 8;
    uint64_t : 8;
    uint64_t Checksum : 8;
} BMS2;

typedef struct __attribute__((packed)) {
    uint64_t ACVoltage : 16;
    uint64_t ACPower: 16;
    uint64_t : 24;
    uint64_t Checksum : 8;
} BMS3;

void BMS_Init();

BMS1 const* BMS_GetData1();
BMS2 const* BMS_GetData2();
BMS3 const* BMS_GetData3();

void BMS_UpdateData1(BMS1* data);
void BMS_UpdateData2(BMS2* data);
void BMS_UpdateData3(BMS3* data);