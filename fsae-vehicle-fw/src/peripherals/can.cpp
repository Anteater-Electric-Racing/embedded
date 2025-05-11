// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1

#include "can.h"
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>

#include "utils/utils.h"

#include "vehicle/motor.h"

#define CAN_INSTANCE CAN1
#define CAN_BAUD_RATE 500000

#define mVCU1_ID 0x101
#define mVCU1_LEN 8 // bytes

#define mBMS1_ID 0x1A0
#define mBMS1_LEN 8 // bytes

typedef struct __attribute__((packed)) {
    uint64_t sMotorTorqueReq : 8; // end of byte 0
    uint64_t sMotorSpdReq : 16; // end of byte 1 and 2
    uint64_t sChangeGearAlarm: 1;
    uint64_t sAuthenticationSts : 2;
    uint64_t Reserved : 5; // end of byte 3
    uint64_t sGearLevelPos_Sts_F : 1;
    uint64_t sGearLevelPos_Sts : 3;
    uint64_t sMainRelayCmd: 1;
    uint64_t sBrakePedal_Sts: 2;
    uint64_t sVehicleState: 1;  // end of byte 4
    uint64_t sKeyPosition : 2;
    uint64_t sWarningLevelVCU : 2;
    uint64_t sMotorModeReq : 2;
    uint64_t sWorkModeReq : 1;
    uint64_t sAC_ControlCmd : 1; // end of byte 5
    uint64_t sPowerReduceReq : 3;
    uint64_t sAuxRelayCmd: 1;
    uint64_t sVCU1_RC : 4; // end of byte 6
    uint64_t sVCU1_CS : 8; // end of byte 7
} VCU1;

typedef struct __attribute__((packed)) {
    uint64_t sBatChargeSts_F : 1;
    uint64_t sHVLockSts: 1;
    uint64_t sPositiveRelay_FB : 1;
    uint64_t sNegativeRelay_FB : 1;
    uint64_t sBatChargeSts : 2;
    uint64_t sWarningLevelBMS : 2; // end of byte 0
    uint64_t sBatSocValue: 8; // end of byte 1
    uint64_t sBatSohValue: 8; // end of byte 2
    uint64_t sReqMode : 2;
    uint64_t sPrecharge_Finshed_F : 1;
    uint64_t sPrecharge_Finished : 1;
    uint64_t sFastChargeRelay_FB : 1;
    uint64_t sPrechargeRelay_FB : 1;
    uint64_t sCmd_ACDC : 2; // end of byte 3
    uint64_t sRlAdhereDetSts : 1;
    uint64_t sChrgRlAdhereDetSts : 1;
    uint64_t sAllowMaxDischarge : 14; // end of byte 4 and byte 5
    uint64_t sHVLockSts_F: 1;
    uint64_t sResInsulationLow_F : 1;
    uint64_t sBatPackMatchAlarm : 1;
    uint64_t sBatPackCoincidenceAlarm : 1;
    uint64_t sBMS1_RC : 4; // end of byte 6
    uint64_t sBMS1_CS : 8; // end of byte 7
} BMS1;

static uint8_t ComputeChecksum(uint8_t* data, uint8_t length);

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
CAN_message_t motorMsg;
CAN_message_t rx_msg;

void CAN_Init() {
    // Initialize CAN bus
    can3.begin();
    can3.setBaudRate(CAN_BAUD_RATE);
    can3.setTX(DEF);
    can3.setRX(DEF);
    can3.enableFIFO();
    can3.enableFIFOInterrupt();
    can3.enableMBInterrupts();
    can3.setMaxMB(16);
}

void CAN_SendVCU1Message(float torqueValue)
{
    motorMsg.id = mVCU1_ID;
    motorMsg.len = mVCU1_LEN;

    VCU1 vcu1 = {0};

    // Map throttle percentage to uint8 value
    vcu1.sMotorTorqueReq = (uint8_t) LINEAR_MAP(torqueValue, 0.0F, 1.0F, 0.0F, 255.0F);

    memcpy(motorMsg.buf, &vcu1, sizeof(vcu1));
    motorMsg.buf[7] = ComputeChecksum(motorMsg.buf, 8);

    if (can3.write(motorMsg)) {
        Serial.println("VCU1 message sent");
    } else {
        Serial.println("VCU1 message failed to send");
    }
}

// checksum = (byte0 + byte1 + byte2 + byte3 + byte4 + byte5 + byte6) XOR 0xFF
static uint8_t ComputeChecksum(uint8_t* data, uint8_t length) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < length - 1; i++) {
        sum += data[i];
    }
    return sum ^ 0xFF;
}



