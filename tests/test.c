#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minunit.h"
#undef INFINITY
#include "../babeld.h"
#include "../util.h"

#define N_RANDOM_TESTS 1e5

void
test_setup(void)
{
    srand(time(NULL));
}

void
test_tearDown(void)
{
    /* NO OP */
}

MU_TEST(roughly_bounds)
{
    int i;
    char err_lower_bound[100] = {};
    char err_upper_bound[100] = {};

    for (i = 0; i < N_RANDOM_TESTS; i++) {
        int input = rand() % 1024;
        /* if (rand() % 2) { */
        /*     input = -input; */
        /* } */

        int output = roughly(input);

        snprintf(err_lower_bound, sizeof(err_lower_bound),
            "Output of roughly function was too low. Input: %d / Output: %d.", input, output);
        mu_assert(output >= 3 * input / 4, err_lower_bound); 

        snprintf(err_upper_bound, sizeof(err_upper_bound),
            "Output of roughly function was too high. Input: %d / Output: %d.", input, output);
        mu_assert(output <= 5 * input / 4, err_upper_bound);
    }
}

MU_TEST_SUITE(babeld_tests)
{
    MU_SUITE_CONFIGURE(&test_setup, &test_tearDown);
    MU_RUN_TEST(roughly_bounds);
}

int
main(void)
{
    MU_RUN_SUITE(babeld_tests);
    return MU_EXIT_CODE;
}
