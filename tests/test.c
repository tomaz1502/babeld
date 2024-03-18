#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minunit.h"
#undef INFINITY
#include "../babeld.h"
#include "../util.h"

#define N_RANDOM_TESTS 128
#define SEED 42

void
test_setup(void)
{
    srand(SEED);
}

void
test_tearDown(void)
{
    /* NO OP */
}

void
swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

MU_TEST(roughly_test)
{
    int i, input, output, lower_bound, upper_bound;
    char err_lower_bound[100];
    char err_upper_bound[100];

    for (i = 0; i < N_RANDOM_TESTS; i++) {
        input = rand() % 1024;
        if (rand() % 2) {
            input = -input;
        }

        output = roughly(input);
        lower_bound = 3 * input / 4;
        upper_bound = 5 * input / 4;

        if (input < 0) {
            swap(&lower_bound, &upper_bound);
        }

        snprintf(err_lower_bound, sizeof(err_lower_bound),
            "Output of roughly function was too low. Input: %d / Output: %d.", input, output);
        mu_assert(output >= lower_bound, err_lower_bound);

        snprintf(err_upper_bound, sizeof(err_upper_bound),
            "Output of roughly function was too high. Input: %d / Output: %d.", input, output);
        mu_assert(output <= upper_bound, err_upper_bound);
    }

    mu_assert(roughly(1) == 1, "roughly(1) should be 1.");
    mu_assert(roughly(0) == 0, "roughly(0) should be 0.");
}

// NOTE: timeval_minus seems to assume that s1 >= s2.
MU_TEST(timeval_minus_test)
{
    struct timeval *tv1, *tv2, result;
    int i;

    struct test_case {
        struct timeval tv1_val;
        struct timeval tv2_val;
        struct timeval expected;
        const char* err_msg;
    };

    struct test_case tcs[] =
    {
        { {42, 10}, {42, 10}, {0, 0},      "{42, 10} - {42, 10} should be {0, 0}." },
        { {45, 10}, {42, 8},  {3, 2},      "{45, 10} - {42, 8} should be {3, 2}." },
        { {45, 10}, {42, 11}, {2, 999999}, "{45, 10} - {42, 11} should be {2, 999999}." }
    };

    int num_of_cases = sizeof(tcs) / sizeof(struct test_case);
    for(i = 0; i < num_of_cases; i++) {
        tv1 = &tcs[i].tv1_val;
        tv2 = &tcs[i].tv2_val;

        timeval_minus(&result, tv1, tv2);
        
        mu_assert(result.tv_usec == tcs[i].expected.tv_usec, tcs[i].err_msg);
        mu_assert(result.tv_sec == tcs[i].expected.tv_sec, tcs[i].err_msg);
    }
}

MU_TEST_SUITE(babeld_tests)
{
    MU_SUITE_CONFIGURE(&test_setup, &test_tearDown);
    MU_RUN_TEST(roughly_test);
    MU_RUN_TEST(timeval_minus_test);
}

int
main(void)
{
    MU_RUN_SUITE(babeld_tests);
    return MU_EXIT_CODE;
}
