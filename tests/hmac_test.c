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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "test_utilities.h"
#include "../rfc6234/sha.h"
#include "../BLAKE2/ref/blake2.h"

#include "../babeld.h"
#include "../interface.h"
#include "../neighbour.h"
#include "../util.h"
#include "../hmac.h"
#include "../configuration.h"
#include "../kernel.h"

#define PACKET_HEADER_SIZE 4
// TODO: is this correct?
#define MAX_PACKET_BODYLEN 500
#define HMAC_MAX_SIZE 32
#define KEY_MAX_SIZE 64

void compute_hmac_test(void)
{
    unsigned char *src, *dst, *packet_header, *body, *hmac;
    int i, num_of_cases, bodylen;
    struct key *key;
    int hmac_len;

    typedef struct test_case {
        unsigned char src_val[ADDRESS_ARRAY_SIZE];
        unsigned char dst_val[ADDRESS_ARRAY_SIZE];
        unsigned char packet_header_val[PACKET_HEADER_SIZE];
        unsigned char body_val[MAX_PACKET_BODYLEN];
        int bodylen_val;
        struct key key_val;
        unsigned char hmac_expected[HMAC_MAX_SIZE];
    } test_case;

    test_case tcs[] =
    {
        {
            .src_val = {254, 128, 0, 0, 0, 0, 0, 0, 2, 22, 62, 255, 254, 197,
                225, 239},
            .dst_val = {255, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6},
            .packet_header_val = {42, 2, 0, 34},
            .body_val = {8, 10, 0, 0, 0, 0, 255, 255, 105, 131, 255, 255, 4, 6,
                0, 0, 138, 241, 0, 10, 17, 12, 0, 0, 0, 13, 111, 180,
                121, 202, 112, 51, 238, 237},
            .bodylen_val = 34,
            .key_val = {
                .id = "k1",
                .type = AUTH_TYPE_SHA256,
                .len = 64,
                .value =
                    (unsigned char[]) {54, 16,  17, 18, 192, 255, 238, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                .ref_count = 2
            },
            .hmac_expected = {12, 124, 238, 71, 58, 55, 173, 152, 18, 174, 138,
                113, 75, 180, 31, 220, 144, 195, 126, 213, 130,
                199, 97, 20, 69, 93, 210, 180, 41, 147, 141, 49},
        },
        {
            .src_val = {254, 128, 0, 0, 0, 0, 0, 0, 2, 22, 62, 255, 254, 0, 0,
                0},
            .dst_val = {255, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6},
            .packet_header_val = {42, 2, 0, 22},
            .body_val = {4, 6, 0, 0, 47, 84, 1, 144, 17, 12, 0, 0, 0, 14, 57,
                72, 181, 138, 248, 108, 171, 133},
            .bodylen_val = 22,
            .key_val = {
                .id = "k1",
                .type = AUTH_TYPE_BLAKE2S128,
                .len = 32,
                .value =
                    (unsigned char[]){184, 17, 96, 231, 142, 203, 75, 118,
                        42, 213, 55, 90, 176, 66, 15, 104, 19,
                        214, 60, 175, 10, 203, 125, 180, 142,
                        232, 123, 168, 191, 50, 173, 44},
                .ref_count = 2
            },
            .hmac_expected = {237, 164, 28, 31, 153, 50, 126, 166, 67, 195, 21,
                19, 123, 77, 57, 46, 112, 39, 177, 23, 146, 86, 0,
                0, 246, 0, 0, 0, 0, 0, 0, 0}
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

        compute_hmac(src, dst, packet_header, body, bodylen, key, hmac);

        hmac_len = tcs[i].key_val.type == AUTH_TYPE_SHA256 ? 32 : 16;
        if(!babel_check(memcmp(hmac, tcs[i].hmac_expected, hmac_len) == 0)) {
            printf("src: %s\n", str_of_array(src, ADDRESS_ARRAY_SIZE));
            printf("dst: %s\n", str_of_array(dst, ADDRESS_ARRAY_SIZE));
            printf("packet_header: %s\n", str_of_array(packet_header, PACKET_HEADER_SIZE));
            printf("body: %s\n", str_of_array(body, bodylen));
            printf("bodylen: %d\n", bodylen);
            printf("key value: %s\n", str_of_array(key->value, key->len));
            printf("hmac computed: %s\n", str_of_array(hmac, hmac_len));
            printf("hmac expected: %s\n", str_of_array(tcs[i].hmac_expected, hmac_len));
        }
    }
}

void setup(void)
{
    protocol_port = 6696;
}

void run_hmac_tests(void)
{
    struct timespec start, end;
    double diff_secs;
    printf("-----------------------------------------------------------\n");
    printf("Running hmac.c tests:\n");
    printf("-----------------------------------------------------------\n");
    setup();
    clock_gettime(CLOCK_MONOTONIC, &start);
    run_test(compute_hmac_test, "compute_hmac_test");
    clock_gettime(CLOCK_MONOTONIC, &end);
    diff_secs = end.tv_sec - start.tv_sec;
    diff_secs += (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("hmac.c tests done.\n");
    printf("Time taken: %.8f seconds.\n", diff_secs);
}
