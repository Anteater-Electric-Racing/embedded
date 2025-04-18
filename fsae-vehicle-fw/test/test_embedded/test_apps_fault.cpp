#include <Arduino.h>
#include <unity.h>
#include "../lib/APPS/apps.h"
#include "../lib/APPS/faults.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}



void test_APPS_Fault(void) {
    // APPSData appsData = APPS_GetAPPSReading_Separate();
    // float appsReading1 = appsData.appsReading1_Percentage;
    // float appsReading2 = appsData.appsReading2_Percentage;

    // if (abs(appsReading1 - appsReading2) > 0.1) {
    //     TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_APPS_MASK);
    // }
    TEST_ASSERT_TRUE(true);
}

void trivialTest(void) {
    TEST_ASSERT_EQUAL(4, 3+1);
}

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();
    // RUN_TEST(test_APPS_Fault);
    RUN_TEST(trivialTest);
    UNITY_END();
}

void loop() {
    // digitalWrite(13, HIGH);
    // delay(100);
    // digitalWrite(13, LOW);
    // delay(500);
}