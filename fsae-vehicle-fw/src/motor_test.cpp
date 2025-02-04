#include <FlexCAN_T4.h>

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

FlexCAN_T4<CAN_INSTANCE, RX_SIZE_256, TX_SIZE_16> CANbus;

void MotorTest_Init()
{
	Serial.begin(115200);
	CANbus.begin();
	CANbus.setBaudRate(CAN_BAUD_RATE);
	Serial.println("CANbus init");
}

// checksum=(byte0+byte1+byte2+byte3+byte4+byte5+byte6) XOR 0xFF
uint8_t ComputeChecksum(uint8_t* data, uint8_t length) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < length - 1; i++) {
        sum += data[i];
    }
    return sum ^ 0xFF;
}

void Motor_SendVCU1Message(uint8_t torqueValue)
{
	CAN_message_t msg;

	msg.id = VCU1_ID;
	msg.len = VCU1_LEN;

	VCU1 vcu1 = {0};
	vcu1.TorqueReq = static_cast<uint8_t>(round(torqueValue / 0.392));

	memcpy(msg.buf, &vcu1, sizeof(vcu1));
	msg.buf[7] = ComputeChecksum(msg.buf, 8);

	if (CANbus.write(msg)) {
		Serial.println("VCU1 message sent");
	} else {
		Serial.println("VCU1 message failed to send");
	}
}

// int main()
// {
// 	MotorTest_Init();
// 	Motor_SendVCU1Message(50);

// 	while (true) {}
// }
