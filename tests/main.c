#include "util_test.h"
#include "hmac_test.h"

int
main(void) {
    run_util_tests();
    run_hmac_tests();

    return 0;
}
