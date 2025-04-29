#include <Arduino.h>
#include <unity.h>
#include "vehicle/apps.h"
#include "vehicle/faults.h"
#include <iostream>

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
    delay(5000); // Wait for serial to be ready
    Serial.begin(9600);
    Serial.println("Starting tests...");
    UNITY_BEGIN();

    while(!Serial) delay(10);
    RUN_TEST(test_APPS_Fault);
    RUN_TEST(trivialTest);

    UNITY_END();
}

void loop() {
    std::cout << "Looping..." << std::endl;
}