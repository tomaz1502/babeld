#include "test_utilities.h"
#include "util_test.h"
#include "hmac_test.h"

int
main(void) {
    tests_run = 0;
    tests_failed = 0;
    run_util_tests();
    run_hmac_tests();
    return 0;
}
