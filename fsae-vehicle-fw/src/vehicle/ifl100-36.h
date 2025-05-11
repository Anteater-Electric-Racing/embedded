#include <stdint.h>

#define mVCU1_ID 0x101
#define mVCU1_LEN 8 // bytes

#define mMCU1_ID 0x105
#define mMCU2_ID 0x106
#define mMCU3_ID 0x107

#define mBMS1_ID 0x1A0


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
    uint64_t sMotorSpeed : 16; // end of byte 0 and byte 1
    uint64_t sMotorTorque: 8; // end of byte 2
    uint64_t sMaxMotorTq : 8; // end of byte 3
    uint64_t sMaxMotorBrakeTq : 8; // end of byte 4
    uint64_t sMotorDirection : 2;
    uint64_t sMotorMainState : 3;
    uint64_t Reserved: 3; // end of byte 5
    uint64_t sWorkMode: 2; // end of byte 2
    uint64_t sMotorMode : 2;
    uint64_t sMCU1_RC : 4; // end of byte 6
    uint64_t sMCU1_CS : 8; // end of byte 7
} MCU1;

typedef struct __attribute__((packed)) {
    uint64_t sMotorTemp : 8; // end of byte 0
    uint64_t sHardwareTemp: 8; // end of byte 1
    uint64_t sDC_OverVolt_Fault : 1;
    uint64_t sMotorOvrHot_Fault : 1;
    uint64_t sMotorOvrSpd_Fault : 1;
    uint64_t sPhaseCurSensor_Fault : 1;
    uint64_t sResolver_Fault: 1;
    uint64_t sOverHot_Fault: 1;
    uint64_t sMotorPhaseCur_Fault : 1;
    uint64_t sDC_OverCur_Fault : 1; // end of byte 2
    uint64_t sV12LowVolt_Fault : 1;
    uint64_t sDC_LowVolt_Fault : 1;
    uint64_t sDC_VoltSensor_Fault : 1;
    uint64_t sMotorTempSensor_Fault : 1;
    uint64_t sTempSensor_Fault : 1;
    uint64_t sMotorSystem_Fault : 1;
    uint64_t sMotorOvrCool_Fault : 1;
    uint64_t Reserved : 17; // end of byte 3, byte 4, and byte 5
    uint64_t sMotorStall_Fault : 1;
    uint64_t sMotorOpenPhase_Fault : 1;
    uint64_t sWarningLevelMCU : 1;
    uint64_t sMCU2_RC : 4; // end of byte 6
    uint64_t sMCU2_CS : 8; // end of byte 7
} MCU2;

typedef struct __attribute__((packed)) {
    uint64_t sDC_Voltage : 16; // end of byte 0 and byte 1
    uint64_t sDC_Current: 16; // end of byte 2 and byte 3
    uint64_t sMotorPhaseCurrent : 16; // end of byte 4 and byte 5
    uint64_t Reserved : 16; // end of byte 6 and byte 7
} MCU3;

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

uint8_t ComputeChecksum(uint8_t* data, uint8_t length);
