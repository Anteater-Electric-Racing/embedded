// Anteater Electric Racing, 2025

#include <unity.h>
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include <iostream>

void test_BSE_Init(void) {
    BSEData* bseData = BSE_GetBSEReading();

    TEST_ASSERT_EQUAL(0, bseData->bseFront_PSI);
}

void test_BSE_UpdateData(void) {
    BSE_UpdateData(100, 200);

    BSEData* bseData = BSE_GetBSEReading();
    float bseReading1 = bseData->bseFront_PSI;
    float bseReading2 = bseData->bseRear_PSI;

    TEST_ASSERT_NOT_EQUAL_FLOAT(0, bseReading1);
    TEST_ASSERT_NOT_EQUAL_FLOAT(0, bseReading2);
}

void test_BSE_OSC_Fault(void) {
    BSE_UpdateData(0, 200);

    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_BSE_MASK);
}

int run_BSE_tests() {
    RUN_TEST(test_BSE_Init);
    RUN_TEST(test_BSE_UpdateData);
    RUN_TEST(test_BSE_OSC_Fault);
    return 0;
}