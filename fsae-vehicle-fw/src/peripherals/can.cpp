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
#define VCU1_ID 0x101
#define VCU1_LEN 8 // bytes

typedef struct {
	uint64_t TorqueReq : 8;
	uint64_t MotorSpdReq : 16;
	uint64_t ChangeGearAlarm: 1;
	uint64_t VCUAuthStatus : 2;
	uint64_t Reserved : 5;
	uint64_t VehicleState: 1;
	uint64_t Brake_Pedal_State: 2;
	uint64_t BMS_Main_Relay_Cmd: 1;
	uint64_t GearLevelPos_Sts : 3;
	uint64_t GearLevelPos_Sts_F : 1;
	uint64_t AC_Control_Cmd : 1;
	uint64_t VCU_WorkMode : 1;
	uint64_t VCU_MotorMode : 2;
	uint64_t VCU_Warning_Level : 2;
	uint64_t KeyPosition : 2;
	uint64_t BMS_Aux_Relay_Cmd: 1;
	uint64_t PowerReduceReq : 3;
	uint64_t RollingCounter : 4;
	uint64_t Checksum : 8;
} VCU1;

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
	motorMsg.id = VCU1_ID;
	motorMsg.len = VCU1_LEN;

	VCU1 vcu1 = {0};

    // Map throttle percentage to uint8 value
	vcu1.TorqueReq = (uint8_t) LINEAR_MAP(torqueValue, 0.0F, 1.0F, 0.0F, 255.0F);

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



