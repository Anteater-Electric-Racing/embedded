// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1
#define THREAD_CAN_TELEMETRY_PRIORITY 2

#define THREAD_TEST_STACK_SIZE 128
#define THREAD_TEST_PRIORITY_BASE 3
#define NUM_TEST_THREADS 5  // Number of test threads to create

#include "can.h"
#include <FlexCAN_T4.h>
#include <isotp.h>
#include <arduino_freertos.h>
#include <semphr.h>

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
CAN_message_t msg;
CAN_message_t rx_msg;

SemaphoreHandle_t xSemaphore;

// TODO: Update IDs to be more logical values
const uint32_t canid = 0x666;
const uint32_t request = 0x777;
CANTelemetryData telemetryDataCAN;
isotp<RX_BANKS_16, 512> tp;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2; // TODO: Figure out actual values for send and recieve

void threadCAN( void *pvParameters );
void threadTelemetryCAN( void *pvParameters );

void CAN_TelemetryInit() {
    xSemaphore = xSemaphoreCreateMutex();
    // fill with reasonable default values
    telemetryDataCAN = {
        .accumulatorVoltage = 0,
        .accumulatorTemp = 0,
        .tractiveSystemVoltage = 0
    };
    // TODO: Initialize adc reads data with more helpful default values
    uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1] = {0, 0, 0, 0, 0, 0, 0, 0};
    // CAN_SerializeTelemetryData(telemetryDataCAN, ); // May be problematic if we forget to call this somewhere after updating data
}

void CAN_Init() {
    // Initialize CAN bus
    can3.begin();
    can3.setBaudRate(500000);
    can3.setTX(DEF);
    can3.setRX(DEF);
    can3.enableFIFO();
    can3.enableFIFOInterrupt();
    can3.enableMBInterrupts();
    can3.setMaxMB(16);

    CAN_TelemetryInit();
    can2.begin();
    can2.setBaudRate(1000000);
    can2.setTX(DEF);
    can2.setRX(DEF);
    can2.enableFIFO();
    can2.enableFIFOInterrupt();
    can2.setMaxMB(16);
    tp.begin();
    tp.setWriteBus(&can2); /* we write to this bus */
}

/* Test code variables */

void threadHighFrequencySend(void *pvParameters);
void threadRandomDelaySend(void *pvParameters);
void threadBurstSend(void *pvParameters);
void threadPriorityInversion(void *pvParameters);
void threadMonitor(void *pvParameters);

// Global test variables
volatile uint32_t messagesSent = 0;
volatile uint32_t messagesReceived = 0;
volatile bool testRunning = false;
SemaphoreHandle_t xTestSemaphore;
TaskHandle_t testTasks[NUM_TEST_THREADS];
uint32_t TEST_DURATION = 30000;


void CAN_Begin() {
    // xTaskCreate(threadCAN, "threadCAN", THREAD_CAN_STACK_SIZE, NULL, THREAD_CAN_PRIORITY, NULL);
    xTaskCreate(threadTelemetryCAN, "threadTelemetryCAN", THREAD_CAN_STACK_SIZE, NULL, THREAD_CAN_TELEMETRY_PRIORITY, NULL);

    xTestSemaphore = xSemaphoreCreateMutex();

    // Start the test
    testRunning = true;

    // Create test threads with different priorities
    xTaskCreate(threadHighFrequencySend, "TestHighFreq", THREAD_TEST_STACK_SIZE,
                NULL, THREAD_TEST_PRIORITY_BASE, &testTasks[0]);

    xTaskCreate(threadRandomDelaySend, "TestRandom", THREAD_TEST_STACK_SIZE,
                NULL, THREAD_TEST_PRIORITY_BASE + 1, &testTasks[1]);

    xTaskCreate(threadBurstSend, "TestBurst", THREAD_TEST_STACK_SIZE,
                NULL, THREAD_TEST_PRIORITY_BASE + 2, &testTasks[2]);

    xTaskCreate(threadPriorityInversion, "TestPriority", THREAD_TEST_STACK_SIZE,
                NULL, THREAD_TEST_PRIORITY_BASE - 1, &testTasks[3]);

    // Monitor thread with highest priority to observe results
    xTaskCreate(threadMonitor, "TestMonitor", THREAD_TEST_STACK_SIZE * 2,
                NULL, THREAD_TEST_PRIORITY_BASE + 3, &testTasks[4]);
}


void threadCAN(void *pvParameters){
    while(true){
        msg.id = 0x123; // Set the CAN ID
        msg.len = 8; // Set the length of the message
        // int res = can3.write(MB0, msg)
        int res = can3.write(msg);
        // can3.mailboxStatus();
        Serial.println(res);
        msg.id++;
        Serial.println("CAN thread running...");
        vTaskDelay(100);
    }
}

void threadTelemetryCAN(void *pvParameters){
    static uint32_t sendTimer;
    sendTimer = millis();
    uint8_t serializedTelemetryBuf[sizeof(CANTelemetryData)];
    while(true){
        if ( millis() - sendTimer > 1000 ) {
                Serial.print("Semaphor take successful");
                xSemaphoreTake(xSemaphore, (TickType_t) 1000);
                CAN_SerializeTelemetryData(telemetryDataCAN, serializedTelemetryBuf);
                xSemaphoreGive(xSemaphore);

                ISOTP_data config;
                config.id = 0x666;
                config.flags.extended = 0; /* standard frame */
                config.separation_time = 10; /* time between back-to-back frames in millisec */
                tp.write(config, serializedTelemetryBuf, sizeof(serializedTelemetryBuf));

                sendTimer = millis();
        }
    }
}

void floatToBytes(float val, uint8_t* buf) {
    memcpy(buf, &val, sizeof(float));
}

void CAN_SerializeTelemetryData(CANTelemetryData data, uint8_t* serializedTelemetryBuf){
    memcpy(serializedTelemetryBuf, &data, sizeof(data));
}

void CAN_UpdateTelemetryData(CANTelemetryData* data) {
    telemetryDataCAN = *data;
}

void CAN_UpdateAccumulatorData(float accumulatorVoltage, float accumulatorTemp, float tractiveSystemVoltage){
    // xSemaphoreTake(xSemaphore, (TickType_t) 1000);
    telemetryDataCAN.accumulatorVoltage = accumulatorVoltage;
    telemetryDataCAN.accumulatorTemp = accumulatorTemp;
    telemetryDataCAN.tractiveSystemVoltage = tractiveSystemVoltage;
    // xSemaphoreGive(xSemaphore);
}

void CAN_UpdateTelemetryADCData(volatile uint16_t* adc0reads, volatile uint16_t* adc1reads){
    xSemaphoreTake(xSemaphore, (TickType_t) 1000);
    memcpy(telemetryDataCAN.adc0Reads, (const uint16_t*)adc0reads, sizeof(telemetryDataCAN.adc0Reads));
    memcpy(telemetryDataCAN.adc1Reads, (const uint16_t*)adc1reads, sizeof(telemetryDataCAN.adc1Reads));
    xSemaphoreGive(xSemaphore);
}

/* Add the testing thread implementation */

// High frequency sender - sends messages at a very rapid rate
void threadHighFrequencySend(void *pvParameters) {
    CAN_message_t msg;
    uint32_t msgCount = 0;

    msg.id = 0x200;  // Different ID range for test messages
    msg.len = 8;

    while (testRunning) {
        // Populate message with test data
        msg.buf[0] = 0xA1;
        msg.buf[1] = 0xB2;
        msg.buf[2] = 0xC3;
        msg.buf[3] = (msgCount >> 24) & 0xFF;
        msg.buf[4] = (msgCount >> 16) & 0xFF;
        msg.buf[5] = (msgCount >> 8) & 0xFF;
        msg.buf[6] = msgCount & 0xFF;
        msg.buf[7] = 0xFF;

        // Send the message
        if (can3.write(msg)) {
            if (xSemaphoreTake(xTestSemaphore, (TickType_t)10) == pdTRUE) {
                messagesSent++;
                xSemaphoreGive(xTestSemaphore);
            }
        }

        msgCount++;
        // Very small delay to create high bus load
        vTaskDelay(1);
    }

    vTaskDelete(NULL);
}

// Random delay sender - simulates unpredictable timing
void threadRandomDelaySend(void *pvParameters) {
    CAN_message_t msg;
    uint32_t msgCount = 0;

    msg.id = 0x300;  // Different ID range
    msg.len = 8;

    while (testRunning) {
        // Random delay between 5-50ms
        vTaskDelay(5 + (rand() % 46));

        // Populate message with identifiable data
        msg.buf[0] = 0xD4;
        msg.buf[1] = 0xE5;
        msg.buf[2] = 0xF6;
        msg.buf[3] = (msgCount >> 24) & 0xFF;
        msg.buf[4] = (msgCount >> 16) & 0xFF;
        msg.buf[5] = (msgCount >> 8) & 0xFF;
        msg.buf[6] = msgCount & 0xFF;
        msg.buf[7] = 0xEE;

        // Send the message
        if (can3.write(msg)) {
            if (xSemaphoreTake(xTestSemaphore, (TickType_t)10) == pdTRUE) {
                messagesSent++;
                xSemaphoreGive(xTestSemaphore);
            }
        }

        msgCount++;
    }

    vTaskDelete(NULL);
}

// Burst sender - simulates burst traffic patterns
void threadBurstSend(void *pvParameters) {
    CAN_message_t msg;
    uint32_t msgCount = 0;

    msg.id = 0x400;  // Different ID range
    msg.len = 8;

    while (testRunning) {
        // Send a burst of 10 messages
        for (int i = 0; i < 10 && testRunning; i++) {
            msg.buf[0] = 0x1A;
            msg.buf[1] = 0x2B;
            msg.buf[2] = 0x3C;
            msg.buf[3] = i;  // Burst index
            msg.buf[4] = (msgCount >> 16) & 0xFF;
            msg.buf[5] = (msgCount >> 8) & 0xFF;
            msg.buf[6] = msgCount & 0xFF;
            msg.buf[7] = 0xDD;

            if (can3.write(msg)) {
                if (xSemaphoreTake(xTestSemaphore, (TickType_t)10) == pdTRUE) {
                    messagesSent++;
                    xSemaphoreGive(xTestSemaphore);
                }
            }

            // Short delay between burst messages
            vTaskDelay(1);
        }

        msgCount++;
        // Longer delay between bursts
        vTaskDelay(200);
    }

    vTaskDelete(NULL);
}

// Priority inversion test - holds semaphore while doing work
void threadPriorityInversion(void *pvParameters) {
    CAN_message_t msg;
    CANTelemetryData testData;
    uint8_t serializedBuffer[sizeof(CANTelemetryData)];

    msg.id = 0x500;
    msg.len = 8;

    while (testRunning) {
        // Take the main telemetry semaphore to create potential priority inversion
        if (xSemaphoreTake(xSemaphore, (TickType_t)1000) == pdTRUE) {
            // Create fake telemetry data
            testData.accumulatorVoltage = 400.0 + (rand() % 100) / 10.0;
            testData.accumulatorTemp = 25.0 + (rand() % 300) / 10.0;
            testData.tractiveSystemVoltage = 600.0 + (rand() % 50) / 10.0;

            // Simulate some processing time while holding the semaphore
            CAN_SerializeTelemetryData(testData, serializedBuffer);

            // Delay while holding semaphore to force priority inversion
            vTaskDelay(rand() % 20);

            // Release the semaphore
            xSemaphoreGive(xSemaphore);

            // Send a test message showing we completed the work
            msg.buf[0] = 0x99;
            msg.buf[1] = 0x88;
            msg.buf[2] = 0x77;
            msg.buf[3] = 0x66;
            msg.buf[4] = 0x55;
            msg.buf[5] = 0x44;
            msg.buf[6] = 0x33;
            msg.buf[7] = 0x22;

            if (can3.write(msg)) {
                if (xSemaphoreTake(xTestSemaphore, (TickType_t)10) == pdTRUE) {
                    messagesSent++;
                    xSemaphoreGive(xTestSemaphore);
                }
            }
        }

        // Delay before next attempt
        vTaskDelay(50 + (rand() % 100));
    }

    vTaskDelete(NULL);
}

// CAN message reception callback - add this function to your code
void canSniff(const CAN_message_t &msg) {
    // Only count our test messages (ID ranges we defined)
    if (msg.id >= 0x200 && msg.id <= 0x500) {
        if (xSemaphoreTake(xTestSemaphore, (TickType_t)10) == pdTRUE) {
            messagesReceived++;
            xSemaphoreGive(xTestSemaphore);
        }
    }
}

// Monitor thread to check system performance and end the test
void threadMonitor(void *pvParameters) {
    uint32_t startTime = millis();
    uint32_t lastTime = startTime;
    uint32_t lastMessagesSent = 0;
    uint32_t lastMessagesReceived = 0;
    char buffer[100];

    // Set up CAN message reception for test messages
    can3.onReceive(canSniff);

    while (testRunning) {
        uint32_t currentTime = millis();

        // Check if we should end the test
        if (currentTime - startTime >= TEST_DURATION) {
            testRunning = false;
            Serial.println("Test duration complete");
            break;
        }

        // Print statistics every second
        if (currentTime - lastTime >= 1000) {
            uint32_t currentSent = 0;
            uint32_t currentReceived = 0;

            if (xSemaphoreTake(xTestSemaphore, (TickType_t)10) == pdTRUE) {
                currentSent = messagesSent;
                currentReceived = messagesReceived;
                xSemaphoreGive(xTestSemaphore);
            }

            uint32_t sentDelta = currentSent - lastMessagesSent;
            uint32_t receivedDelta = currentReceived - lastMessagesReceived;
            float lossPercentage = 0;

            if (sentDelta > 0) {
                lossPercentage = 100.0 * (1.0 - ((float)receivedDelta / sentDelta));
            }

            // Report the status
            sprintf(buffer, "Time: %ds, Messages sent: %lu/s, received: %lu/s, loss: %.2f%%",
                    (currentTime - startTime) / 1000, sentDelta, receivedDelta, lossPercentage);
            Serial.println(buffer);

            // Check CPU load and stack usage for each task
            for (int i = 0; i < NUM_TEST_THREADS; i++) {
                UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(testTasks[i]);
                sprintf(buffer, "Task %d stack free: %lu words", i, stackHighWaterMark);
                Serial.println(buffer);
            }

            // Store for next comparison
            lastMessagesSent = currentSent;
            lastMessagesReceived = currentReceived;
            lastTime = currentTime;
        }

        vTaskDelay(100);  // Check 10 times per second
    }

    // Test is complete, print final results
    uint32_t totalSent = 0;
    uint32_t totalReceived = 0;

    if (xSemaphoreTake(xTestSemaphore, (TickType_t)100) == pdTRUE) {
        totalSent = messagesSent;
        totalReceived = messagesReceived;
        xSemaphoreGive(xTestSemaphore);
    }

    Serial.println("=== CAN Bus Stress Test Results ===");
    sprintf(buffer, "Total messages sent: %lu", totalSent);
    Serial.println(buffer);
    sprintf(buffer, "Total messages received: %lu", totalReceived);
    Serial.println(buffer);

    float lossRate = 0;
    if (totalSent > 0) {
        lossRate = 100.0 * (1.0 - ((float)totalReceived / totalSent));
    }
    sprintf(buffer, "Overall message loss rate: %.2f%%", lossRate);
    Serial.println(buffer);

    // Clean up - delete all test tasks
    for (int i = 0; i < NUM_TEST_THREADS - 1; i++) {  // -1 because we're deleting ourselves last
        if (testTasks[i] != NULL) {
            vTaskDelete(testTasks[i]);
        }
    }

    Serial.println("Test complete. All test threads terminated.");
    vTaskDelete(NULL);  // Delete this monitor task
}


// Add a function to manually trigger the test from your main code
void startCANStressTest(uint32_t duration) {
    if (!testRunning) {
        // Update the test duration if needed
        TEST_DURATION = duration;

        // Create the test semaphore if it doesn't exist
        if (xTestSemaphore == NULL) {
            xTestSemaphore = xSemaphoreCreateMutex();
        }

        // Reset counters
        if (xSemaphoreTake(xTestSemaphore, (TickType_t)10) == pdTRUE) {
            messagesSent = 0;
            messagesReceived = 0;
            xSemaphoreGive(xTestSemaphore);
        }

        // Start the test
        testRunning = true;

        // Create test threads with different priorities
        xTaskCreate(threadHighFrequencySend, "TestHighFreq", THREAD_TEST_STACK_SIZE,
                   NULL, THREAD_TEST_PRIORITY_BASE, &testTasks[0]);

        xTaskCreate(threadRandomDelaySend, "TestRandom", THREAD_TEST_STACK_SIZE,
                   NULL, THREAD_TEST_PRIORITY_BASE + 1, &testTasks[1]);

        xTaskCreate(threadBurstSend, "TestBurst", THREAD_TEST_STACK_SIZE,
                   NULL, THREAD_TEST_PRIORITY_BASE + 2, &testTasks[2]);

        xTaskCreate(threadPriorityInversion, "TestPriority", THREAD_TEST_STACK_SIZE,
                   NULL, THREAD_TEST_PRIORITY_BASE - 1, &testTasks[3]);

        // Monitor thread with highest priority to observe results
        xTaskCreate(threadMonitor, "TestMonitor", THREAD_TEST_STACK_SIZE * 2,
                   NULL, THREAD_TEST_PRIORITY_BASE + 3, &testTasks[4]);

    }
}
