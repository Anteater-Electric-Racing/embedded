#include <Arduino.h>
#include <FlexCAN_T4.h>

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

void setup() {
    can2.begin();
    can2.setBaudRate(1000000);
    can2.setTX(DEF);
    can2.setRX(DEF);
    can2.enableFIFO();
    can2.enableFIFOInterrupt();
    can2.setMaxMB(16);

    Serial.begin(115200);
    Serial.println("CAN dump started...");
}

void loop() {
    CAN_message_t msg;
    if (can2.read(msg)) {
        Serial.print("ID: 0x");
        Serial.print(msg.id, HEX);
        Serial.print("  LEN: ");
        Serial.print(msg.len);
        Serial.print("  DATA: ");
        for (uint8_t i = 0; i < msg.len; i++) {
            Serial.print(msg.buf[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}
