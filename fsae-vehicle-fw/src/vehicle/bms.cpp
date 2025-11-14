#include <vehicle/bms.h>
#include <peripherals/can.h>
#include <arduino_freertos.h>

#define batteryStatusID 0x02
#define BPS_StatusID 0x03
#define errorsID 0x01

BMSBatteryStatus batteryStatus;
BMS_BPSStatus BPS_Status;
BMSErrors errors;
static uint32_t rx_idBMS;
static uint64_t rx_dataBMS;

void BMS_Init() {
    batteryStatus = {
        .dataPackCurrent = 0,
        .dataPackVoltage = 0,
        .batteryPct = 0,
        .highestTempC = 0,
        .avgTempC = 0
    };

    BPS_Status = {
        .relay_status = 0,
        .isolation_status = 0,
        .flags = {}
    };

    errors = {
        .error1 = 0,
        .error2 = 0,
        .error3 = 0
    };
}

static void BMSThread(void *pvParameters) {
    for(;;) {
        CAN_Receive(&rx_idBMS, &rx_dataBMS);
        switch (rx_idBMS) {
        case batteryStatusID:
            memcpy(&batteryStatus, &rx_dataBMS, sizeof(batteryStatus));
            break;
        case errorsID:
            memcpy(&errors, &rx_dataBMS, sizeof(errors));
            break;
        case BPS_StatusID:
            memcpy(&BPS_Status, &rx_dataBMS, sizeof(BPS_Status));
            break;
        default:
            break;
        }

    }
}
