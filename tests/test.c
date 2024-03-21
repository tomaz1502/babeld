#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

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

    typedef struct test_case {
        struct timeval tv1_val;
        struct timeval tv2_val;
        struct timeval expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {42, 10}, {42, 10}, {0, 0},      "{42, 10} - {42, 10} should be {0, 0}." },
        { {45, 10}, {42, 8},  {3, 2},      "{45, 10} - {42, 8} should be {3, 2}." },
        { {45, 10}, {42, 11}, {2, 999999}, "{45, 10} - {42, 11} should be {2, 999999}." }
    };

    int num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        tv1 = &tcs[i].tv1_val;
        tv2 = &tcs[i].tv2_val;

        timeval_minus(&result, tv1, tv2);
        
        mu_assert(result.tv_usec == tcs[i].expected.tv_usec, tcs[i].err_msg);
        mu_assert(result.tv_sec == tcs[i].expected.tv_sec, tcs[i].err_msg);
    }
}

MU_TEST(timeval_minus_msec_test)
{
    struct timeval *tv1, *tv2;
    int i, num_of_cases;
    unsigned result;

    typedef struct test_case {
        struct timeval tv1_val;
        struct timeval tv2_val;
        unsigned expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {42, 10}, {42, 10}, 0, "{42, 10} - {42, 10} should be 0ms." },
        { {100, 20000}, {40, 5000}, 60015, "{100, 20000} - {40, 5000} should be 60015ms." },
        { {100, 20000}, {40, 5001}, 60014, "{100, 20000} - {40, 5001} should be 60014ms." },
        { {100, 20000}, {100, 19000}, 1, "{100, 20000} - {100, 19000} should be 1ms." },
        { {100, 20000}, {101, 19000}, 0, "{100, 20000} - {101, 19000} should be 0ms." },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        tv1 = &tcs[i].tv1_val;
        tv2 = &tcs[i].tv2_val;

        result = timeval_minus_msec(tv1, tv2);

        mu_assert(result == tcs[i].expected, tcs[i].err_msg);
    }
}

MU_TEST(timeval_add_msec_test)
{
    struct timeval *tv1, result;
    int msecs, num_of_cases, i;

    typedef struct test_case {
        struct timeval tv1_val;
        int msecs_val;
        struct timeval expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {42, 10}, 50, { 42, 50010 }, "{42, 10} + 50ms should be {42, 50010}." },
        { {42, 990000}, 10, { 43, 0 }, "{42, 990000} + 10ms should be {43, 0}." },
        { {42, 990000}, 20, { 43, 10000 }, "{42, 990000} + 20ms should be {43, 10000}." },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        tv1 = &tcs[i].tv1_val;
        msecs = tcs[i].msecs_val;

        timeval_add_msec(&result, tv1, msecs);

        mu_assert(result.tv_sec == tcs[i].expected.tv_sec, tcs[i].err_msg);
        mu_assert(result.tv_usec == tcs[i].expected.tv_usec, tcs[i].err_msg);
    }
}

MU_TEST(timeval_compare_test)
{
    struct timeval *s1, *s2;
    int result, i, num_of_cases;

    typedef struct test_case {
        struct timeval s1_val;
        struct timeval s2_val;
        int expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {42, 10}, {42, 10}, 0, "{42, 10} should be equal to {42, 10}." },
        { {42, 10}, {42, 50}, -1, "{42, 10} should be smaller than {42, 50}." },
        { {42, 50}, {42, 10}, 1, "{42, 50} should greater than {42, 10}." },
        { {42, 10}, {52, 10}, -1, "{42, 10} should be smaller than {52, 10}." },
        { {52, 10}, {42, 10}, 1, "{52, 10} should be greater than {42, 10}." },
        { {52, 10}, {42, 5}, 1, "{52, 10} should be greater than {42, 5}." },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        s1 = &tcs[i].s1_val;
        s2 = &tcs[i].s2_val;

        result = timeval_compare(s1, s2);

        mu_assert(result == tcs[i].expected, tcs[i].err_msg);
    }
}

MU_TEST(timeval_min_test)
{
    struct timeval *s1, *s2;
    int i, num_of_cases;

    typedef struct test_case {
        struct timeval s1_val;
        struct timeval s2_val;
        struct timeval expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {42, 10}, {42, 10}, {42, 10}, "timeval_min({42, 10}, {42, 10}) should be {42, 10}." },
        { {42, 10}, {0, 0}, {42, 10}, "timeval_min({42, 10}, {0, 0}) should be {42, 10}." },
        { {0, 0}, {42, 10}, {42, 10}, "timeval_min({0, 0}, {42, 10}) should be {42, 10}." },
        { {42, 9}, {42, 10}, {42, 9}, "timeval_min({42, 9}, {42, 10}) should be {42, 9}." },
        { {41, 15}, {42, 10}, {41, 15}, "timeval_min({41, 15}, {42, 10}) should be {41, 15}." },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        s1 = &tcs[i].s1_val;
        s2 = &tcs[i].s2_val;

        timeval_min(s1, s2);

        mu_assert(s1->tv_sec == tcs[i].expected.tv_sec, tcs[i].err_msg);
        mu_assert(s1->tv_usec == tcs[i].expected.tv_usec, tcs[i].err_msg);
    }
}

MU_TEST(timeval_min_sec_test)
{
    struct timeval *s;
    time_t secs;
    int i, num_of_cases;

    typedef struct test_case {
        struct timeval s_val;
        time_t secs_val;
        time_t secs_expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {42, 10}, 41, 41, "timeval_min_sec({42, 10}, 41) should be {41, _}." },
        { {42, 10}, 43, 42, "timeval_min_sec({42, 10}, 41) should be {42, _}." },
        // NOTE: Is it correct? Infinity shouldn't be just {0, 0}?
        { {0, 10}, 1024, 1024, "timeval_min_sec({0, 10}, 1024) should be {1024, _}." },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        s = &tcs[i].s_val;
        secs = tcs[i].secs_val;

        timeval_min_sec(s, secs);

        mu_assert(s->tv_sec == tcs[i].secs_expected, tcs[i].err_msg);
    }
}

MU_TEST(parse_nat_test)
{
    const char *string;
    int result, i, num_of_cases;

    typedef struct test_case {
        const char *string_val;
        int expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { "42", 42, "parse_nat(\"42\") should be 42." },
        { "212348123481293", -1, "parse_nat(\"212348123481293\") should be -1." },
        { "0", 0, "parse_nat(\"0\") should be 0." },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        string = tcs[i].string_val;

        result = parse_nat(string);

        mu_assert(result == tcs[i].expected, tcs[i].err_msg);
    }
}

MU_TEST(parse_thousands_test)
{
    const char *string;
    int result, i, num_of_cases;

    typedef struct test_case {
        const char * const string_val;
        int expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { "42.1337", 42133, "parse_thousands(\"42.1337\") should be 42133." },
        { "10.123456", 10123, "parse_thousands(\"10.123456\") should be 10123." },
        { "0.1", 100, "parse_thousands(\"0.1\") should be 100." },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        string = tcs[i].string_val;

        result = parse_thousands(string);

        mu_assert(result == tcs[i].expected, tcs[i].err_msg);
    }
}

MU_TEST(h2i_test)
{
    char c;
    int result, i, num_of_cases;

    typedef struct test_case {
        char c_val;
        int expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { '1', 1, "h2i('1') should be 1." },
        { '9', 9, "h2i('9') should be 9." },
        { 'A', 10, "h2i('A') should be 10." },
        { 'C', 12, "h2i('C') should be 12." },
        { 'd', 13, "h2i('d') should be 13." },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        c = tcs[i].c_val;

        result = h2i(c);

        mu_assert(result == tcs[i].expected, tcs[i].err_msg);
    }
}

MU_TEST(fromhex_test)
{
    unsigned char *dst;
    const char *src;
    int n, i, num_of_cases, dst_len;

// TODO: Is there a better way?
#define EXPECTED_MAX_SIZE 42

    typedef struct test_case {
        unsigned char expected[EXPECTED_MAX_SIZE];
        const char *src_val;
        int n_val;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {255},         "ff",     2, "fromhex(\"ff\") should be {255}." },
        { {238, 171},    "eeab",   4, "fromhex(\"eeab\") should be {238, 171}." },
        { {10, 42, 200}, "0A2aC8", 6, "fromhex(\"0A2aC8\") should be {10, 42, 200}." },
    };

#define DST_MAX_SIZE 42

    dst = malloc(DST_MAX_SIZE * sizeof(unsigned char));

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        src = tcs[i].src_val;
        n = tcs[i].n_val;

        dst_len = fromhex(dst, src, n);

        int dst_eq_expected = memcmp(dst, tcs[i].expected, dst_len) == 0;
        mu_assert(dst_eq_expected, tcs[i].err_msg);
    }
}

// NOTE: Skipping do_debugf

MU_TEST(in_prefix_test)
{
    const unsigned char *restrict prefix, *restrict address;
    unsigned char plen;
    int num_of_cases, i, result;

#define PREFIX_MAX_SIZE 42
#define ADDRESS_MAX_SIZE 42

    typedef struct test_case {
        const unsigned char prefix_val[PREFIX_MAX_SIZE];
        const unsigned char address_val[ADDRESS_MAX_SIZE];
        unsigned char plen_val;
        int expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {42, 42}, {42, 42, 42}, 16, 1, "{42, 42} should be contained in the prefix of {42, 42, 42} of length 16 bits." },
        { {42, 21}, {42, 42, 42}, 16, 0, "{42, 21} should not be contained in the prefix of {42, 42, 42} of length 16 bits." },
        { {1, 2, 254}, {1, 2, 255}, 23, 1, "{1, 2, 254} should be contained in the prefix of {1, 2, 255} of length 23 bits." }
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        prefix = tcs[i].prefix_val;
        address = tcs[i].address_val;
        plen = tcs[i].plen_val;

        result = in_prefix(prefix, address, plen);

        mu_assert(result == tcs[i].expected, tcs[i].err_msg);
    }
}

MU_TEST(normalize_prefix_test)
{
    const unsigned char *restrict prefix;
    unsigned char *restrict result;
    unsigned char plen;
    int num_of_cases, i, j;

#define PREFIX_MAX_SIZE 42
#define EXPECTED_MAX_SIZE 42
#define RESULT_MAX_SIZE 42

    typedef struct test_case {
        unsigned char expected[EXPECTED_MAX_SIZE];
        const unsigned char prefix_val[PREFIX_MAX_SIZE];
        unsigned char plen_val;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {4, 6}, {4, 6}, 16, "normalize_prefix(ret, {4, 6}, 16) should be {4, 6}." },
        { {1, 2, 252}, {1, 2, 255}, 22, "normalize_prefix(ret, {1, 2, 255}, 22) should be {1, 2, 252}." },
        { {1, 1, 1, 1}, {1, 1, 1, 0}, 30, "normalize_prefix(ret, {1, 1, 1, 1}, 30) should be {1, 1, 1, 0}." },
    };

    result = malloc(RESULT_MAX_SIZE * sizeof(unsigned char));

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        prefix = tcs[i].prefix_val;
        plen = tcs[i].plen_val;

        // NOTE: I do not understand the usage of `memset` in this function
        normalize_prefix(result, prefix, plen);

        int result_ok = memcmp(result, tcs[i].expected, plen / 8) == 0;

        for(j = 0; j < plen % 8; j++) {
            int mask = 1 << (8 - j);
            int bit_ok = (result[plen / 8] & mask) ==
                         (tcs[i].expected[plen / 8] & mask);
            result_ok &= bit_ok;
        }
        mu_assert(result_ok, tcs[i].err_msg);
    }
}

MU_TEST(format_address_test)
{
    const unsigned char *address;
    const char *result;
    int num_of_cases, i;

#define ADDRESS_MAX_SIZE 42
#define RESULT_MAX_SIZE 42

    typedef struct test_case {
        unsigned char address_val[ADDRESS_MAX_SIZE];
        const char *expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255},
          "fffe:782a:140f:370c:5a63:5505:c896:78ff",
          "format_address({255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255}) should be \"fffe:782a:140f:370c:5a63:5505:c896:78ff\"."
        },
        { {170, 170, 187, 187, 204, 204},
          "aaaa:bbbb:cccc::",
          "format_address({170, 170, 187, 187, 204, 204}) should be \"aaaa:bbbb:cccc::\"."
        },
        { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 128,0,0,1},
          "128.0.0.1",
          "format_address({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 128, 0, 0, 1}) should be \"128.0.0.1\"."
        },
    };

    result = malloc(RESULT_MAX_SIZE * sizeof(unsigned char));

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        address = tcs[i].address_val;

        result = format_address(address);

        mu_assert(strcmp(result, tcs[i].expected) == 0, tcs[i].err_msg);
    }
}

MU_TEST_SUITE(babeld_tests)
{
    MU_SUITE_CONFIGURE(&test_setup, &test_tearDown);
    MU_RUN_TEST(roughly_test);
    MU_RUN_TEST(timeval_minus_test);
    MU_RUN_TEST(timeval_minus_msec_test);
    MU_RUN_TEST(timeval_add_msec_test);
    MU_RUN_TEST(timeval_compare_test);
    MU_RUN_TEST(timeval_min_test);
    MU_RUN_TEST(timeval_min_sec_test);
    MU_RUN_TEST(parse_nat_test);
    MU_RUN_TEST(parse_thousands_test);
    MU_RUN_TEST(h2i_test);
    MU_RUN_TEST(fromhex_test);
    MU_RUN_TEST(in_prefix_test);
    MU_RUN_TEST(normalize_prefix_test);
    MU_RUN_TEST(format_address_test);
}

int
main(void)
{
    MU_RUN_SUITE(babeld_tests);
    putchar('\n');
    return MU_EXIT_CODE;
}
