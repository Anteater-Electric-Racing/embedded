// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1

#include "stdint.h"
#include "can.h"
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>

#include "utils/utils.h"

#include "vehicle/motor.h"
#include "vehicle/ifl100-36.h"

#define CAN_INSTANCE CAN1
#define CAN_BAUD_RATE 500000

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
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

    can2.begin();
    can2.setBaudRate(CAN_BAUD_RATE);
    can2.setTX(DEF);
    can2.setRX(DEF);
    can2.enableFIFO();
}

void CAN_SendVCU1Message(uint32_t id, uint64_t msg)
{
    motorMsg.id = id;

    memcpy(motorMsg.buf, &msg, sizeof(msg));
    
    if (can2.write(motorMsg)) {
        Serial.println("CAN message sent");
    } else {
        Serial.println("CAN message failed to send");
    }
}

void CAN_Receive() {
    if (can2.read(rx_msg)) {
        if (rx_msg.id == mMCU1_ID) {
            // MCU1 mcu1 = {0};
            // memcpy(&mcu1, rx_msg.buf, sizeof(mcu1));
            // Serial.println("MCU1 message received");
            // Serial.print("Motor speed: ");
            // Serial.println(mcu1.sMotorSpeed);
            // Serial.print("Motor torque: ");
            // Serial.println(mcu1.sMotorTorque);
            // Serial.print("Max motor torque: ");
            // Serial.println(mcu1.sMaxMotorTq);
            // Serial.print("Max motor brake torque: ");
            // Serial.println(mcu1.sMaxMotorBrakeTq);
            // Serial.print("Motor direction: ");
            // Serial.println(mcu1.sMotorDirection);
            // Serial.print("Motor main state: ");
            // Serial.println(mcu1.sMotorMainState);
            // Serial.print("MCU1 work mode: ");
            // Serial.println(mcu1.sWorkMode);
            // Serial.print("MCU1 motor mode: ");
            // Serial.println(mcu1.sMotorMode);
        } else if (rx_msg.id == mMCU2_ID) {
            // MCU2 mcu2 = {0};
            // memcpy(&mcu2, rx_msg.buf, sizeof(mcu2));
            // Serial.println("MCU2 message received -----------------------------");
            // Serial.print("motor temp: ");
            // Serial.println(mcu2.sMotorTemp);
            // Serial.print("hardware temp: ");
            // Serial.println(mcu2.sHardwareTemp);
            // Serial.print("DC over voltage fault: ");
            // Serial.println(mcu2.sDC_OverVolt_Fault);
            // Serial.print("motor overheat fault: ");
            // Serial.println(mcu2.sMotorOvrHot_Fault);
            // Serial.print("motor over speed fault: ");
            // Serial.println(mcu2.sMotorOvrSpd_Fault);
            // Serial.print("phase current sensor fault: ");
            // Serial.println(mcu2.sPhaseCurSensor_Fault);
            // Serial.print("resolver fault: ");
            // Serial.println(mcu2.sResolver_Fault);
            // Serial.print("overheat fault: ");
            // Serial.println(mcu2.sOverHot_Fault);
            // Serial.print("motor phase current fault: ");
            // Serial.println(mcu2.sMotorPhaseCur_Fault);
            // Serial.print("DC over current fault: ");
            // Serial.println(mcu2.sDC_OverCur_Fault);
            // Serial.print("V12 low voltage fault: ");
            // Serial.println(mcu2.sV12LowVolt_Fault);
            // Serial.print("DC low voltage fault: ");
            // Serial.println(mcu2.sDC_LowVolt_Fault);
            // Serial.print("DC voltage sensor fault: ");
            // Serial.println(mcu2.sDC_VoltSensor_Fault);
            // Serial.print("motor temperature sensor fault: ");
            // Serial.println(mcu2.sMotorTempSensor_Fault);
            // Serial.print("temperature sensor fault: ");
            // Serial.println(mcu2.sTempSensor_Fault);
            // Serial.print("motor system fault: ");
            // Serial.println(mcu2.sMotorSystem_Fault);
            // Serial.print("motor overcool fault: ");
            // Serial.println(mcu2.sMotorOvrCool_Fault);
            // Serial.print("motor stall fault: ");
            // Serial.println(mcu2.sMotorStall_Fault);
            // Serial.print("motor open phase fault: ");
            // Serial.println(mcu2.sMotorOpenPhase_Fault);
            // Serial.print("warning level MCU: ");
            // Serial.println(mcu2.sWarningLevelMCU);
        } else if (rx_msg.id == mMCU3_ID) {
            // MCU3 mcu3 = {0};
            // memcpy(&mcu3, rx_msg.buf, sizeof(mcu3));
            // Serial.println("MCU3 message received -----------------------------");
            // Serial.print("DC voltage: ");
            // Serial.println(mcu3.sDC_Voltage);
            // Serial.print("DC current: ");
            // Serial.println(mcu3.sDC_Current);
            // Serial.print("motor phase current: ");
            // Serial.println(mcu3.sMotorPhaseCurrent);
        } else {
            Serial.print("Unknown CAN message ID: ");
            Serial.println(rx_msg.id);
        }
    }
}