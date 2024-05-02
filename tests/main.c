#include "test_utilities.h"
#include "util_test.h"
#include "hmac_test.h"

int
main(void) {
    tests_run = 0;
    tests_failed = 0;
    run_suite(util_test_suite, "util.c");
    run_suite(hmac_test_suite, "hmac.c");
    return 0;
}
