// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1

#include <stdint.h>
#include <isotp.h>
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>

#include "peripherals/can.h"
#include "utils/utils.h"
#include "vehicle/motor.h"
#include "vehicle/ifl100-36.h"

#define CAN_INSTANCE CAN1
#define CAN_BAUD_RATE 500000

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
isotp<RX_BANKS_16, 512> tp;

CAN_message_t motorMsg;
CAN_message_t rx_msg;

// CAN loss detection
// static uint32_t lastCAN2MsgTime = 0;
// static uint32_t lastCAN3MsgTime = 0;
static bool can2Healthy = false;
static bool can3Healthy = false;

#define CAN_TIMEOUT_MS 100


//CAN SETUP
/**
 * CAN2 (CAN2 on diagram) --> ORION, CCM, RASPI, PCC
 * CAN3 (CAN1 on diagram) --> CCM, OMNI, RASPI
 *
 */


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
    // can3.enableFIFOInterrupt();
    // can3.setMaxMB(16); // Set maximum message buffers to 16

    tp.begin();
    tp.setWriteBus(&can3); // Set the bus to write to can3
}

//TODO @ksthakkar: make can_send general function to select both CANbuses or make seperate CAN_send functions
//can2
void CAN_Send(uint32_t id, uint64_t msg)
{
    motorMsg.id = id;
   // msg.id = 0x100;
    memcpy(motorMsg.buf, &msg, sizeof(msg));

    can2.write(motorMsg);

    Serial.println("| Sending CAN... ");
    Serial.print("");
}

void CAN_Receive(uint32_t* rx_id, uint64_t* rx_data) {
    if (can2.read(rx_msg)) {
        *rx_id = rx_msg.id;
        memcpy(rx_data, rx_msg.buf, sizeof(*rx_data));

        Serial.println("--> CAN Message recieved");
        // lastCAN2MsgTime = millis();
        // can2Healthy = true;
    } else { // No message received, assign default values
        *rx_id = 0;
        *rx_data = 0;
    }
}

void CAN_CheckHealth() {
    // uint32_t now = millis();
    // if (now - lastCAN2MsgTime > CAN_TIMEOUT_MS) {
    //     can2Healthy = false;
    // }

    // if (now - lastCAN3MsgTime > CAN_TIMEOUT_MS) {
    //     can3Healthy = false;
    // }
}


void CAN_ISOTP_Send(uint32_t id, uint8_t* msg, uint16_t size) {
    ISOTP_data config;
    config.id = id;
    config.flags.extended = 0; // Standard frame
    config.separation_time = 1; // Time between back-to-back frames in milliseconds
    tp.write(config, msg, size);
}


bool CAN_IsBusHealthy(uint8_t bus) {
    if (bus == 2) return can2Healthy;
    if (bus == 3) return can3Healthy;
    return false;
}
