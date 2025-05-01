// Anteater Electric Racing, 2025

#include <unity.h>
#include "vehicle/apps.h"
#include "vehicle/faults.h"
#include <iostream>

void test_APPS_Init(void) {
    APPSData appsData = APPS_GetAPPSReading_Separate();

    TEST_ASSERT_EQUAL(0, appsData.appsReading1_Percentage);
    TEST_ASSERT_EQUAL(0, appsData.appsReading2_Percentage);
}

void test_APPS_UpdateData(void) {
    APPS_UpdateData(100, 200);

    APPSData appsData = APPS_GetAPPSReading_Separate();
    float appsReading1 = appsData.appsReading1_Percentage;
    float appsReading2 = appsData.appsReading2_Percentage;

    TEST_ASSERT_NOT_EQUAL_FLOAT(0, appsReading1);
    TEST_ASSERT_NOT_EQUAL_FLOAT(0, appsReading2);
}

void test_APPS_Fault_0(void) {
    APPS_UpdateData(0, 0); // 0% difference

    TEST_ASSERT_FALSE(*Faults_GetFaults() & FAULT_APPS_MASK);
}

void test_APPS_Fault_09(void) {
    APPS_UpdateData(0, 85); // 9% difference

    TEST_ASSERT_FALSE(*Faults_GetFaults() & FAULT_APPS_MASK);
}

void test_APPS_Fault_10(void) {
    APPS_UpdateData(0, 95); // 10% difference

    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_APPS_MASK);
}

void test_APPS_Brake_Plausibility_Fault(void) {
    BSE_UpdateData(10000, 10000); // 100 PSI // TODO: Update with accurate value
    APPS_UpdateData(720, 1027);
    // APPS module checks this fault so BSE has to be updated first

    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_APPS_BRAKE_PLAUSIBILITY_MASK);
}

int run_APPS_tests() {
    RUN_TEST(test_APPS_Init);
    RUN_TEST(test_APPS_UpdateData);
    RUN_TEST(test_APPS_Fault_0);
    RUN_TEST(test_APPS_Fault_09);
    RUN_TEST(test_APPS_Fault_10);
    RUN_TEST(test_APPS_Brake_Plausibility_Fault);
    return 0;
}