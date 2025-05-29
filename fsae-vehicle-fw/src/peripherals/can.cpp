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

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;

CAN_message_t motorMsg;
CAN_message_t rx_msg;

void CAN_Init() {
    // Initialize CAN bus
    can2.begin();
    can2.setBaudRate(CAN_BAUD_RATE);
    can2.setTX(DEF);
    can2.setRX(DEF);
    can2.enableFIFO();

    can3.begin();
    can3.setBaudRate(CAN_BAUD_RATE);
    can3.setTX(DEF);
    can3.setRX(DEF);
    can3.enableFIFO();
    can3.enableFIFOInterrupt();
    can3.enableMBInterrupts();
    can3.setMaxMB(16);
}

void CAN_Send(uint32_t id, uint64_t msg)
{
    motorMsg.id = id;

    memcpy(motorMsg.buf, &msg, sizeof(msg));

    # if DEBUG_FLAG
        if (can3.write(motorMsg)) {
            Serial.println("VCU1 message sent");
            Serial.print("Torque: ");
            Serial.println(vcu1.TorqueReq);
            Serial.print("Motor message buf: ");
            for (int i = 0; i < 8; ++i){
                Serial.print(motorMsg.buf[i]);
                Serial.print(" ");
            }
            Serial.println();


        } else {
            Serial.println("VCU1 message failed to send");
        }
    # endif
}

void CAN_Receive(uint32_t* rx_id, uint64_t* rx_data) {
    if (can2.read(rx_msg)) {
        *rx_id = rx_msg.id;
        memcpy(rx_data, rx_msg.buf, sizeof(*rx_data));
    } else {
        *rx_id = 0; // No message received
        *rx_data = 0;
    }
}
