#include "ifl100-36.h"

#include "arduino_freertos.h"

#include "peripherals/can.h"

#include "vehicle/telemetry.h"

#include "utils/utils.h"

#include "pcc_receive.h"

/**
 * Helper to pack J1939 data into the uint64_t used by CAN_Send
 */
uint64_t PackIMDData(uint8_t index, uint16_t value) {
    uint8_t buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    buf[0] = index;
    buf[1] = (uint8_t)(value & 0xFF);        // Low Byte (Intel)
    buf[2] = (uint8_t)((value >> 8) & 0xFF); // High Byte (Intel)

    uint64_t packed;
    memcpy(&packed, buf, sizeof(uint64_t));
    return packed;
}

/**
 * @brief Updates the IMD isolation error threshold
 * @param threshold_kOhm Value between 30 and 2000
 */
void IMD_UpdateThreshold(uint16_t threshold_kOhm) {
    if (threshold_kOhm < 30 || threshold_kOhm > 2000)
        return;

    // 1. Unlock: Index 0x6B, Value 0xFC
    CAN_Send(IMD_CMD_ID, PackIMDData(IMD_IDX_STATUS_LOCK, IMD_LOCK_UNLOCK));
    vTaskDelay(pdMS_TO_TICKS(100));

    // 2. Set Threshold: Index 0x47, Value threshold_kOhm
    CAN_Send(IMD_CMD_ID, PackIMDData(IMD_IDX_THRESHOLD_ERROR, threshold_kOhm));
    vTaskDelay(pdMS_TO_TICKS(100));

    // 3. Relock: Index 0x6B, Value 0xFD
    CAN_Send(IMD_CMD_ID, PackIMDData(IMD_IDX_STATUS_LOCK, IMD_LOCK_LOCK));
}

// PGN_Request (0xEFgg) with Index 0x46 to read Threshold Error
void IMD_RequestThresholdReadback() {
    uint8_t msg[8] = {0x46, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint64_t packed;
    memcpy(&packed, msg, 8);
    CAN_Send(IMD_CMD_ID, packed);
}

void IMD_InitializeConfiguration() {
    // Formula for FSAE: Pack Voltage * 500 Ohms/Volt
    // Example: 400V * 500 = 200,000 Ohms = 200 kOhm
    uint16_t ruleThreshold = 260; // Set to 250k for a safety margin

    Serial.println("IMD: Starting configuration...");

    // 1. Send the Update (Unlock -> Write -> Lock)
    IMD_UpdateThreshold(ruleThreshold);

    // 2. Wait for the IMD to finish its internal flash cycle
    vTaskDelay(pdMS_TO_TICKS(500));

    // 3. Request a readback to verify it "stuck"
    IMD_RequestThresholdReadback();
}
