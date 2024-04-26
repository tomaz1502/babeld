/*
Copyright (c) 2024 by Tomaz Mascarenhas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdlib.h>
#include <netinet/in.h>

#include "minunit.h"
#include "../rfc6234/sha.h"
#include "../BLAKE2/ref/blake2.h"

#undef INFINITY
#include "../babeld.h"
#include "../interface.h"
#include "../neighbour.h"
#include "../util.h"
#include "../hmac.h"
#include "../configuration.h"

#define ERR_MSG_MAX_SIZE 700
#define ADDRESS_ARRAY_SIZE 16
#define PACKET_HEADER_SIZE 4
// TODO: is this correct?
#define MAX_PACKET_BODYLEN 500
#define HMAC_MAX_SIZE 32
#define KEY_MAX_SIZE 64
#define ARR_MAX_SIZE 200

// Duplicated for now
// TODO: create test_utilities.h
char*
str_of_array2(const unsigned char* const arr, size_t len) {
    static char str[4][ARR_MAX_SIZE];
    static int i = 0;
    size_t j;
    int pt;

    i = (i + 1) % 4;

    pt = sprintf(str[i], "{ ");
    for(j = 0; j < len - 1; ++j) {
        pt += sprintf(str[i] + pt, "0x%02x, ", arr[j]);
    }
    pt += sprintf(str[i] + pt, "0x%02x }", arr[len - 1]);
    str[i][pt] = '\0';

    return str[i];
}

void
hmac_test_setup(void)
{
	protocol_port = 6696;
}

void
hmac_test_tearDown(void)
{
    /* NO OP */
}

MU_TEST(compute_hmac_test)
{
	unsigned char *src, *dst, *packet_header, *body, *hmac;
	int i, num_of_cases, bodylen;
	char err_msg[ERR_MSG_MAX_SIZE];
	struct key *key;
	int hmac_len, rc;

	typedef struct test_case {
		unsigned char src_val[ADDRESS_ARRAY_SIZE];
		unsigned char dst_val[ADDRESS_ARRAY_SIZE];
		unsigned char packet_header_val[PACKET_HEADER_SIZE];
		unsigned char body_val[MAX_PACKET_BODYLEN];
		int bodylen_val;
		struct key key_val;
		unsigned char hmac_expected[HMAC_MAX_SIZE];
	} test_case;

	// TODO: What do we do here? This should be inside the table, but we can't put it directly since key.value was defined as a pointer
	unsigned char key_val1[KEY_MAX_SIZE] =
	  {54, 16,  17, 18, 192, 255, 238, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	test_case tcs[] =
	{
		{
			.src_val = {254, 128, 0, 0, 0, 0, 0, 0, 2, 22, 62, 255, 254, 197, 225, 239},
			.dst_val = {255, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6},
			.packet_header_val = {42, 2, 0, 34},
			.body_val = {8, 10, 0, 0, 0, 0, 255, 255, 105, 131, 255, 255, 4, 6, 0, 0, 138, 241, 0, 10, 17, 12, 0, 0, 0, 13, 111, 180, 121, 202, 112, 51, 238, 237},
			.bodylen_val = 34,
			.key_val = { .id = "42", .type = AUTH_TYPE_SHA256, .len = 64, .value = key_val1, .ref_count = 2 },
			.hmac_expected = {12, 124, 238, 71, 58, 55, 173, 152, 18, 174, 138, 113, 75, 180, 31, 220, 144, 195, 126, 213, 130, 199, 97, 20, 69, 93, 210, 180, 41, 147, 141, 49},
		}
	};

	hmac = malloc(HMAC_MAX_SIZE * sizeof(unsigned char));

    num_of_cases = sizeof(tcs) / sizeof(test_case);
	for(i = 0; i < num_of_cases; ++i) {
		src = tcs[i].src_val;
		dst = tcs[i].dst_val;
		packet_header = tcs[i].packet_header_val;
		body = tcs[i].body_val;
		bodylen = tcs[i].bodylen_val;
		key = &tcs[i].key_val;

		rc = compute_hmac(src, dst, packet_header, body, bodylen, key, hmac);
		printf("rc = %d\n", rc);

		sprintf(
			err_msg,
			"\ncompute_hmac() = %s\nexpected hmac: %s.",
			str_of_array2(hmac, 32),
			str_of_array2(tcs[i].hmac_expected, 32)
		);
		hmac_len = tcs[i].key_val.type == AUTH_TYPE_SHA256 ? 32 : 16;
		mu_assert(memcmp(hmac, tcs[i].hmac_expected, hmac_len) == 0, err_msg);
	}
}

MU_TEST_SUITE(hmac_test_suite)
{
	MU_SUITE_CONFIGURE(&hmac_test_setup, &hmac_test_tearDown);
	MU_RUN_TEST(compute_hmac_test);
	MU_REPORT();
}

int
run_hmac_tests(void) {
    printf("--------------------------------------------\n");
    printf("Running tests for hmac.c.\n");
    printf("--------------------------------------------\n");
    MU_RUN_SUITE(hmac_test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
