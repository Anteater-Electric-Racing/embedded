#include <unity.h>
#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "../test_desktop/test_apps_fault.h"
#include "../test_desktop/test_bse_fault.h"

void setUp(void) {
    // set stuff up here
    APPS_Init();
    BSE_Init();
    Faults_Init();
}

void tearDown(void) {
    // clean stuff up here
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    run_APPS_tests();
    run_BSE_tests();

    UNITY_END();
    return 0;
}