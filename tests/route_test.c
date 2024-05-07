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
#include <time.h>
#include <arpa/inet.h>

#include "test_utilities.h"
#include "../babeld.h"
#include "../interface.h"
#include "../source.h"
#include "../route.h"

void route_compare_test(void) {
    int i, num_of_cases, rc;
    unsigned char *prefix, *src_prefix;
    unsigned char plen, src_plen;
    struct babel_route route;

    typedef struct test_case {
        unsigned char *prefix_val;
        unsigned char plen_val;
        unsigned char *src_prefix_val;
        unsigned char src_plen_val;
        unsigned char *route_src_prefix_val;
        unsigned char route_src_plen_val;
        unsigned char *route_prefix_val;
        unsigned char route_plen_val;
        int expected_rc;
    } test_case;

    test_case tcs[] =
    {
        {
            .prefix_val = (unsigned char[])
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 10, 0, 3, 227, 128, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 255, 255, 0, 0, 0, 0, 96, 235, 53, 96, 0, 1, 0, 74, 17, 0, 0, 0, 0, 0, 0,
                 81, 0, 0, 0, 0, 0, 0, 0, 0, 224, 199, 76, 113, 91, 0, 0, 224, 172, 225, 195, 128,
                 112, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 254, 255,
                 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0},
            .plen_val = 128,
            .src_prefix_val = (unsigned char[])
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 96, 235, 53, 96, 0, 1, 0, 74,
                 17, 0, 0, 0, 0, 0, 0, 81, 0, 0, 0, 0, 0, 0, 0, 0, 224, 199, 76, 113, 91, 0, 0, 224,
                 172, 225, 195, 128, 112, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
                 0, 0, 0, 254, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            .src_plen_val = 96,
            .route_src_prefix_val = (unsigned char[])
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 96, 235, 53, 96, 0, 1, 0, 74,
                 17, 0, 0, 0, 0, 0, 0, 81, 0, 0, 0, 0, 0, 0, 0, 0, 224, 199, 76, 113, 91, 0, 0, 224,
                 172, 225, 195, 128, 112, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
                 0, 0, 0, 254, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            .route_src_plen_val = 96,
            .route_prefix_val = (unsigned char[])
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 10, 0, 3, 227, 128, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 255, 255, 0, 0, 0, 0, 96, 235, 53, 96, 0, 1, 0, 74, 17, 0, 0, 0, 0, 0, 0, 81,
                 0, 0, 0, 0, 0, 0, 0, 0, 224, 199, 76, 113, 91, 0, 0, 224, 172, 225, 195, 128, 112,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 254, 255, 255,
                 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0},
            .route_plen_val = 128,
            .expected_rc = 0
        },
        {

            .prefix_val = (unsigned char[])
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 192, 168, 1, 101, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 255, 255, 0, 0, 0, 0, 128, 96, 0, 0, 2, 165, 84, 255, 254, 235, 67, 183, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 192, 168, 1, 101, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 255, 255, 0, 0, 0, 0, 128, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0},
            .plen_val = 128,
            .src_prefix_val = (unsigned char[])
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 128, 96, 0, 0, 2, 165, 84, 255,
                 254, 235, 67, 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 192, 168, 1, 101, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 128, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            .src_plen_val = 96,
            .route_src_prefix_val = (unsigned char[])
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 96, 235, 53, 96, 0, 1, 0, 74,
                 17, 0, 0, 0, 0, 0, 0, 81, 0, 0, 0, 0, 0, 0, 0, 0, 224, 199, 76, 113, 91, 0, 0, 224,
                 172, 225, 195, 128, 112, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
                 0, 0, 0, 254, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            .route_src_plen_val = 96,
            .route_prefix_val = (unsigned char[])
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 10, 0, 3, 227, 128, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 255, 255, 0, 0, 0, 0, 96, 235, 53, 96, 0, 1, 0, 74, 17, 0, 0, 0, 0, 0, 0, 81,
                 0, 0, 0, 0, 0, 0, 0, 0, 224, 199, 76, 113, 91, 0, 0, 224, 172, 225, 195, 128, 112,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 254, 255, 255,
                 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0},
            .route_plen_val = 128,
            .expected_rc = 182
        },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);
    route.src = malloc(sizeof(struct source));
    for(i = 0; i < num_of_cases; ++i) {
        prefix = tcs[i].prefix_val;
        plen = tcs[i].plen_val;
        src_prefix = tcs[i].src_prefix_val;
        src_plen = tcs[i].src_plen_val;
        route.src->plen = tcs[i].route_plen_val;
        memcpy(route.src->prefix, tcs[i].route_prefix_val, tcs[i].route_plen_val);
        route.src->src_plen = tcs[i].route_src_plen_val;
        memcpy(route.src->src_prefix, tcs[i].route_src_prefix_val, tcs[i].route_src_plen_val);

        rc = route_compare(prefix, plen, src_prefix, src_plen, &route);

        if(!babel_check(rc == tcs[i].expected_rc)) {
            fprintf(stderr, "Failed test on route_compare\n");
            fprintf(stderr, "prefix: %s\n", str_of_array(prefix, plen));
            fprintf(stderr, "plen: %d\n", plen);
            fprintf(stderr, "src_prefix: %s\n", str_of_array(src_prefix, src_plen));
            fprintf(stderr, "src_plen: %d\n", src_plen);
            fprintf(stderr, "route->src->prefix: %s\n", str_of_array(route.src->prefix, route.src->plen));
            fprintf(stderr, "route->src->plen: %d\n", route.src->plen);
            fprintf(stderr, "expected rc: %d\n", tcs[i].expected_rc);
            fprintf(stderr, "computed rc: %d\n", rc);
            fflush(stderr);
        }
    }
    free(route.src);
}

void route_test_suite() {
    run_test(route_compare_test, "route_compare_test");
}
