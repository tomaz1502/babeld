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
    int i, num_of_cases;

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

    num_of_cases = sizeof(tcs) / sizeof(test_case);

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
    unsigned char plen, mask;
    int num_of_cases, i, j, test_ok, bit_ok;

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

        normalize_prefix(result, prefix, plen);

        test_ok = memcmp(result, tcs[i].expected, plen / 8) == 0;

        for(j = 0; j < plen % 8; j++) {
            mask = 1 << (8 - j - 1);
            bit_ok = (result[plen / 8] & mask) ==
                     (tcs[i].expected[plen / 8] & mask);
            test_ok &= bit_ok;
        }
        mu_assert(test_ok, tcs[i].err_msg);
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
        { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 127,0,0,1},
          "127.0.0.1",
          "format_address({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 127, 0, 0, 1}) should be \"127.0.0.1\"."
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

// NOTE: The only purpose of this function is to append "/`plen`" in the result of format_address?
MU_TEST(format_prefix_test)
{
    unsigned char plen, *prefix;
    const char *result;
    int num_of_cases, i;

#define PREFIX_MAX_SIZE 42

    typedef struct test_case {
        unsigned char prefix_val[PREFIX_MAX_SIZE];
        unsigned char plen_val;
        const char *expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255},
          120,
          "fffe:782a:140f:370c:5a63:5505:c896:78ff/120",
          "format_prefix({255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255}, 120) should be \"fffe:782a:140f:370c:5a63:5505:c896:78ff/120\"."
        },
        { {170, 170, 187, 187, 204, 204},
           50,
          "aaaa:bbbb:cccc::/50",
          "format_prefix({170, 170, 187, 187, 204, 204}, 50) should be \"aaaa:bbbb:cccc::/50\"."
        },
        { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 127,0,0,1},
          100,
          "127.0.0.1/4",
          "format_prefix({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 127, 0, 0, 1}, 100) should be \"127.0.0.1/4\"."
        },

    };
    
    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; ++i) {
        plen = tcs[i].plen_val;
        prefix = tcs[i].prefix_val;
        
        result = format_prefix(prefix, plen);

        mu_assert(strcmp(result, tcs[i].expected) == 0, tcs[i].err_msg);
    }
}

MU_TEST(format_eui64_test)
{
    unsigned char *eui;
    const char *result;
    int num_of_cases, i;

#define EUI_MAX_SIZE 8

    typedef struct test_case {
        unsigned char eui_val[EUI_MAX_SIZE];
        const char *expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {255, 254, 120, 42, 20, 15, 55, 12},
          "ff:fe:78:2a:14:0f:37:0c",
          "format_eui64({255, 254, 120, 42, 20, 15, 55, 12}) should be \"ff:fe:78:2a:14:0f:37:0c\"."
        },
        { {170, 170, 187, 187, 204, 204, 221, 221},
          "aa:aa:bb:bb:cc:cc:dd:dd",
          "format_eui64({170, 170, 187, 187, 204, 204, 221, 221}) should be \"aa:aa:bb:bb:cc:cc:dd:dd\"."
        },
    };
    
    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; ++i) {
        eui = tcs[i].eui_val;
        
        result = format_eui64(eui);

        mu_assert(strcmp(result, tcs[i].expected) == 0, tcs[i].err_msg);
    }
}

MU_TEST(format_thousands_test)
{
    unsigned int value;
    const char *result;
    int num_of_cases, i;


    typedef struct test_case {
        unsigned int value_val;
        const char *expected;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { 1024, "1.024", "format_thousands(1024) should be \"1.024\"." },
        { 512, "0.512", "format_thousands(512) should be \"0.512\"" },
        { 1234567, "1234.567", "format_thousands(1234567) should be \"1234.567\"" }
    };
    
    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; ++i) {
        value = tcs[i].value_val;
        
        result = format_thousands(value);

        mu_assert(strcmp(result, tcs[i].expected) == 0, tcs[i].err_msg);
    }
}

MU_TEST(parse_address_test)
{
    char *address;
    unsigned char *addr_r;
    int *af_r;
    int rc, num_of_cases, i, test_ok;

#define ADDR_R_MAX_SIZE 42

    typedef struct test_case {
        char *const address_val;
        unsigned char expected_addr_r[ADDR_R_MAX_SIZE];
        int expected_af_r, expected_rc;
        const char *err_msg;
    } test_case;

    addr_r = malloc(ADDR_R_MAX_SIZE * sizeof(unsigned char));
    af_r = malloc(sizeof(int));

    test_case tcs[] =
    {
        { "fffe:782a:140f:370c:5a63:5505:c896:78ff",
          {255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255},
          AF_INET6,
          0,
          "parse_address(\"fffe:782a:140f:370c:5a63:5505:c896:78ff\") should be {255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255} (AF_INET6)."
        },
        {  "127.0.0.1",
           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 127, 0, 0, 1},
           AF_INET,
           0,
           "parse_address(\"127.0.0.1\") should be {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 127, 0, 0, 1}."
        }
    };
    
    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; ++i) {
        address = tcs[i].address_val;

        rc = parse_address(address, addr_r, af_r);
        
        test_ok = memcmp(addr_r, tcs[i].expected_addr_r, 16) == 0;
        test_ok &= (tcs[i].expected_af_r == *af_r);
        test_ok &= (tcs[i].expected_rc == rc);
        mu_assert(test_ok, tcs[i].err_msg);
    }
}

MU_TEST(parse_net_test)
{
    const char *net;
    unsigned char *prefix_r, *plen_r, mask;
    int *af_r, rc, num_of_cases, i, j, test_ok;

#define NET_MAX_SIZE 64
#define PREFIX_MAX_SIZE 42

    typedef struct test_case {
        char *const net_val;
        unsigned char expected_prefix_r[PREFIX_MAX_SIZE];
        unsigned char expected_plen_r;
        int expected_af_r, expected_rc;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        {
            .net_val           = "default",
            .expected_prefix_r = {},
            .expected_plen_r   = 0,
            .expected_af_r     = AF_INET6,
            .expected_rc       = 0,
            .err_msg           = "parse_net(\"default\") should be all zeroes (AF_INET6)."
        },
        { "127.0.0.1/2",
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 64, 0, 0, 0},
          98,
          AF_INET,
          0,
          "parse_net(\"127.0.0.1/2\") should be {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 64, 0, 0, 0} (AF_INET)." },
        { "127.0.0.1",
           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 127, 0, 0, 1},
           128,
           AF_INET,
           0,
           "parse_net(\"127.0.0.1\") should be {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 127, 0, 0, 1} (AF_INET)."
        },
        { "fffe:782a:140f:370c:5a63:5505:c896:78ff",
          {255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255},
          128,
          AF_INET6,
          0,
          "parse_address(\"fffe:782a:140f:370c:5a63:5505:c896:78ff\") should be {255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255} (AF_INET6)."
        },
        { "fffe:782a:140f:370c:5a63:5505:c896:78ff/60",
          {255, 254, 120, 42, 20, 15, 55, 8, 0, 0, 0, 0, 0, 0, 0, 0},
          60,
          AF_INET6,
          0,
          "parse_address(\"fffe:782a:140f:370c:5a63:5505:c896:78ff\") should be {255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255} (AF_INET6)."
        },
    };


    net = malloc(sizeof(char) * NET_MAX_SIZE);
    prefix_r = malloc(sizeof(unsigned char) * PREFIX_MAX_SIZE);
    plen_r = malloc(sizeof(unsigned char));
    af_r = malloc(sizeof(int));

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; ++i) {
        net = tcs[i].net_val;

        rc = parse_net(net, prefix_r, plen_r, af_r);
    
        test_ok = (*plen_r == tcs[i].expected_plen_r);
        test_ok &= (*af_r == tcs[i].expected_af_r);
        test_ok &= (rc == tcs[i].expected_rc);
        test_ok &= (memcmp(tcs[i].expected_prefix_r, prefix_r, tcs[i].expected_plen_r / 8) == 0);
        for(j = 0; j < tcs[i].expected_plen_r % 8; ++j) {
            mask = 1 << (8 - j - 1);
            test_ok &= ((tcs[i].expected_prefix_r[tcs[i].expected_plen_r / 8] & mask) ==
                        (prefix_r[tcs[i].expected_plen_r / 8] & mask));
        }

        mu_assert(test_ok, tcs[i].err_msg);
    }
}

MU_TEST(parse_eui64_test)
{
    const char *eui;
    unsigned char eui_r[8];
    int i, num_of_cases, test_ok, rc;

    typedef struct test_case {
        char *eui_val;
        unsigned char expected_eui_r[8];
        int expected_rc;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { "ff-34-42-ce-14-1f-ab-cc",
          { 255, 52, 66, 206, 20, 31, 171, 204 },
          0,
          "parse_eui64(\"ff-34-42-ce-14-1f-ab-cc\") should be { 255, 52, 66, 206, 20, 31, 171, 204 }."
        },
        { "0b:ae:3d:31:42:00:ac:c5",
          { 11, 174, 61, 49, 66, 0, 172, 197 },
          0,
          "parse_eui64(\"0b:ae:3d:31:42:00:ac:c5\") should be { 11, 174, 61, 49, 66, 0, 172, 197 }."
        },
        { "0b:ae:3d:31:42:00",
          { 11, 174, 61, 255, 254, 49, 66, 0 },
          0,
          "parse_eui64(\"0b:ae:3d:31:42:00\") should be { 11, 174, 61, 255, 254, 49, 66, 0 }."
        },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; i++) {
        eui = tcs[i].eui_val;

        rc = parse_eui64(eui, eui_r);
        
        test_ok = (rc == tcs[i].expected_rc);
        test_ok &= (memcmp(eui_r, tcs[i].expected_eui_r, 8) == 0);
        mu_assert(test_ok, tcs[i].err_msg);
    }
}

MU_TEST(wait_for_fd_test)
{
    int rc;
    rc = wait_for_fd(1, STDOUT_FILENO, 100);
    mu_assert(rc == 1, "STDOUT should be able to write\n");
}

MU_TEST(martian_prefix_test)
{
    unsigned char *prefix;
    int i, num_of_cases, plen, rc;

#undef PREFIX_MAX_SIZE
#define PREFIX_MAX_SIZE 16

    typedef struct test_case {
        unsigned char prefix_val[PREFIX_MAX_SIZE];
        int plen_val, expected_rc;
        const char *err_msg;
    } test_case;

    test_case tcs[] =
    {
        { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 127, 0, 0, 1},
          104,
          1,
          "martian_prefix({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 127, 0, 0, 1}, 104) should be true."
        },
        { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0xE2, 42, 42, 42},
          100,
          1,
          "martian_prefix({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 224, 42, 42, 42}, 100) should be true."
        },
        { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
          128,
          1,
          "martian_prefix({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 128) should be true."
        },
        { {42, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255},
          128,
          0,
          "martian_prefix({42, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255}, 128) should be false."
        },
        { {255, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255},
          128,
          1,
          "martian_prefix({42, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255}, 128) should be false."
        },
        { {254, 128, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255},
          128,
          1,
          "martian_prefix({42, 254, 120, 42, 20, 15, 55, 12, 90, 99, 85, 5, 200, 150, 120, 255}, 128) should be false."
        },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);

    for(i = 0; i < num_of_cases; ++i) {
        prefix = tcs[i].prefix_val;
        plen = tcs[i].plen_val;

        rc = martian_prefix(prefix, plen);

        mu_assert(rc == tcs[i].expected_rc, tcs[i].err_msg);
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
    MU_RUN_TEST(format_prefix_test);
    MU_RUN_TEST(format_eui64_test);
    MU_RUN_TEST(format_thousands_test);
    MU_RUN_TEST(parse_address_test);
    MU_RUN_TEST(parse_net_test);
    MU_RUN_TEST(parse_eui64_test);
    MU_RUN_TEST(wait_for_fd_test);
    MU_RUN_TEST(martian_prefix_test);
}

int
main(void)
{
    MU_RUN_SUITE(babeld_tests);
    putchar('\n');
    return MU_EXIT_CODE;
}
