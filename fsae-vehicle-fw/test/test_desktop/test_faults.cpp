// Anteater Electric Racing, 2025

#include <unity.h>
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include <iostream>

void test_Faults_Init(void) {
    TEST_ASSERT_EQUAL(0, *Faults_GetFaults());
}

void test_Faults_SetFaults(void) {
    Faults_SetFault(FAULT_OVER_CURRENT);
    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_OVER_CURRENT_MASK);

    Faults_SetFault(FAULT_UNDER_VOLTAGE);
    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_UNDER_VOLTAGE_MASK);

    Faults_SetFault(FAULT_OVER_TEMP);
    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_OVER_TEMP_MASK);

    Faults_SetFault(FAULT_APPS);
    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_APPS_MASK);

    Faults_SetFault(FAULT_BSE);
    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_BSE_MASK);

    Faults_SetFault(FAULT_BPPS);
    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_BPPS_MASK);

    Faults_SetFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_APPS_BRAKE_PLAUSIBILITY_MASK);
}

void test_Faults_MultiFaults(void) {
    Faults_SetFault(FAULT_OVER_CURRENT);
    Faults_SetFault(FAULT_UNDER_VOLTAGE);
    Faults_SetFault(FAULT_APPS);

    uint32_t faults = 1 << FAULT_OVER_CURRENT | 1 << FAULT_UNDER_VOLTAGE | 1 << FAULT_APPS;

    TEST_ASSERT_TRUE(faults & *Faults_GetFaults());
}

void test_Faults_ClearFaults(void) {
    Faults_SetFault(FAULT_OVER_CURRENT);
    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_OVER_CURRENT_MASK);

    Faults_ClearFault(FAULT_OVER_CURRENT);
    TEST_ASSERT_FALSE(*Faults_GetFaults() & FAULT_OVER_CURRENT_MASK);
}

void test_Faults_CheckAllClear(void) {
    Faults_SetFault(FAULT_OVER_CURRENT);
    TEST_ASSERT_FALSE(Faults_CheckAllClear());

    Faults_ClearFault(FAULT_OVER_CURRENT);
    TEST_ASSERT_TRUE(Faults_CheckAllClear());
}

int run_Faults_tests() {
    RUN_TEST(test_Faults_Init);
    RUN_TEST(test_Faults_SetFaults);
    RUN_TEST(test_Faults_MultiFaults);
    RUN_TEST(test_Faults_ClearFaults);
    RUN_TEST(test_Faults_CheckAllClear);
    return 0;
}