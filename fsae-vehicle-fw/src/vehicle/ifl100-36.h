#include <stdint.h>

#define mVCU1_ID 0x101
#define mVCU1_LEN 8 // bytes

#define mMCU1_ID 0x105
#define mMCU2_ID 0x106
#define mMCU3_ID 0x107

#define mBMS1_ID 0x1A0
#define mBMS2_ID 0x1A1


typedef struct __attribute__((packed)) {
    uint64_t VCU_TorqueReq : 8; // end of byte 0
    uint64_t VCU_MotorSpdReq : 16; // end of byte 1 and 2
    uint64_t ChangeGearAlarm: 1;
    uint64_t VCUAuthenticationStatus : 2;
    uint64_t Reserved : 5; // end of byte 3
    uint64_t VehicleState: 1;
    uint64_t Brake_Pedal_Sts: 2;
    uint64_t BMS_Main_Relay_Cmd: 1;
    uint64_t GearLeverPos_Sts : 3;
    uint64_t GearLeverPos_Sts_F : 1;  // end of byte 4
    uint64_t AC_Control_Cmd : 1;
    uint64_t VCU_WorkMode : 1;
    uint64_t VCU_MotorMode : 2;
    uint64_t VCU_Warning_Level : 2;
    uint64_t KeyPosition : 2; // end of byte 5
    uint64_t BMS_Aux_Relay_Cmd: 1;
    uint64_t PowerReduceReq : 3;
    uint64_t RollingCounter : 4; // end of byte 6
    uint64_t CheckSum : 8; // end of byte 7
} VCU1;

typedef struct __attribute__((packed)) {
    uint64_t MCU_ActMotorSpd : 16; // end of byte 0 and byte 1
    uint64_t MCU_ActMotorTq: 8; // end of byte 2
    uint64_t MCU_MaxMotorTq : 8; // end of byte 3
    uint64_t MCU_MaxMotorBrakeTq : 8; // end of byte 4
    uint64_t MCU_MotorRatoteDirection : 2;
    uint64_t Reserved: 3;
    uint64_t MCU_MotorMainState : 3; // end of byte 5
    uint64_t MCU_MotorState : 2;
    uint64_t MCU_MotorWorkMode: 2;
    uint64_t RollingCounter : 4; // end of byte 6
    uint64_t CheckSum : 8; // end of byte 7
} MCU1;

typedef struct __attribute__((packed)) {
    uint64_t MCU_Motor_Temp : 8; // end of byte 0
    uint64_t MCU_hardwareTemp: 8; // end of byte 1
    uint64_t MCU_DC_MainWireOverCurrFault : 1;
    uint64_t MCU_MotorPhaseCurrFault : 1;
    uint64_t MCU_OverHotFault: 1;
    uint64_t sResolver_Fault: 1; // ?? MCU_RotateTransformerFault
    uint64_t MCU_PhaseCurrSensorState : 1;
    uint64_t MCU_MotorOverSpdFault : 1;
    uint64_t Drv_MotorOverHotFault : 1;
    uint64_t MCU_DC_MainWireOverVoltFault : 1; // end of byte 2
    uint64_t Drv_MotorOverCoolFault : 1;
    uint64_t MCU_MotorSystemState : 1;
    uint64_t MCU_TempSensorState : 1;
    uint64_t MCU_MotorTempSensorState : 1;
    uint64_t MCU_DC_VoltSensorState : 1;
    uint64_t MCU_DC_LowVoltWarning : 1;
    uint64_t MCU_12V_LowVoltWarning : 1;
    uint64_t Reserved : 17; // end of byte 3, byte 4, and byte 5
    uint64_t RollingCounter : 4;
    uint64_t MCU_Warning_Level : 2;
    uint64_t MCU_MotorOpenPhaseFault : 1;
    uint64_t MCU_MotorStallFault : 1; // end of byte 6
    uint64_t CheckSum : 8; // end of byte 7
} MCU2;

typedef struct __attribute__((packed)) {
    uint64_t MCU_DC_MainWireVolt : 16; // end of byte 0 and byte 1
    uint64_t MCU_DC_MainWireCurr: 16; // end of byte 2 and byte 3
    uint64_t MCU_MotorPhaseCurr : 16; // end of byte 4 and byte 5
    uint64_t Reserved : 16; // end of byte 6 and byte 7
} MCU3;

typedef struct __attribute__((packed)) {
    uint64_t BMS_Warning_Level : 2;
    uint64_t Batt_charge_Sts : 2;
    uint64_t Negative_Relay_FB : 1;
    uint64_t Positive_Relay_FB : 1;
    uint64_t HighVoltLoopLockSts: 1;
    uint64_t Batt_Charge_Sts_F : 1; // end of byte 0
    uint64_t Batt_SOC_Value: 8; // end of byte 1
    uint64_t Batt_SOH_Value: 8; // end of byte 2
    uint64_t BMS_Cmd_AC_DC : 2;
    uint64_t Pre_charge_Relay_FB : 1;
    uint64_t Fast_charge_Relay_FB : 1;
    uint64_t Pre_charge_Finish_Sts : 1;
    uint64_t Pre_charge_Finish_Sts_F : 1;
    uint64_t BMS_Req_Mode : 2; // end of byte 3
    uint64_t Batt_Pack_Coincidence_Alarm : 1;
    uint64_t Batt_Pack_Matching_Alarm : 1;
    uint64_t Insulation_Resistance_Low_F : 1;
    uint64_t HighVoltLoopLockSts_F: 1;
    uint64_t ChargeRelayAdhereDetectionSts : 1;
    uint64_t RelayAdhereDetectionSts : 1;
    uint64_t BMS_Allow_Max_Discharge : 14; // end of byte 4 and byte 5 and half of byte 6
    uint64_t RollingCounter : 4; // end of byte 6
    uint64_t CheckSum : 8; // end of byte 7
} BMS1;

typedef struct __attribute__((packed)) {
    uint64_t Reserved1 : 8; // end of byte 0
    uint64_t sAllowMaxDischarge : 16; // end of byte 1 and byte 2
    uint64_t sAllowMaxRegenCharge: 16; // end of byte 3 and byte 4
    uint64_t Reserved2 : 8; // end of byte 5
    uint64_t RollingCounter : 4;
    uint64_t Reserved3 : 4; // end of byte 6
    uint64_t CheckSum : 8; // end of byte 7
} BMS2;

uint8_t ComputeChecksum(uint8_t* data, uint8_t length);
