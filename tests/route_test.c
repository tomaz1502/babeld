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
#include "../util.h"
#include "../kernel.h"
#include "../interface.h"
#include "../source.h"
#include "../neighbour.h"
#include "../route.h"
#include "../xroute.h"
#include "../message.h"
#include "../resend.h"
#include "../configuration.h"
#include "../local.h"

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
                { 204, 191, 204, 17, 179, 148, 97, 201, 24, 33, 133, 32, 138, 138, 104, 235 },
            .plen_val = 128,
            .src_prefix_val = (unsigned char[])
                { 167, 145, 127, 130, 201, 185, 216, 226, 87, 1, 78, 203, 236, 64, 33, 184 },
            .src_plen_val = 96,
            .route_src_prefix_val = (unsigned char[])
                { 0, 237, 201, 179, 130, 42, 124, 154, 75, 1, 186, 213, 139, 34, 192, 50 },
            .route_src_plen_val = 96,
            .route_prefix_val = (unsigned char[])
                { 180, 64, 181, 125, 249, 141, 95, 81, 142, 173, 28, 122, 238, 61, 50, 238 },
            .route_plen_val = 128,
            .expected_rc = 24
        },
        {

            .prefix_val = (unsigned char[])
                { 201, 5, 52, 158, 160, 192, 253, 113, 137, 217, 19, 232, 162, 114, 41, 141 },
            .plen_val = 128,
            .src_prefix_val = (unsigned char[])
                { 234, 209, 73, 225, 36, 213, 61, 230, 152, 59, 215, 238, 134, 233, 23, 140 },
            .src_plen_val = 96,
            .route_src_prefix_val = (unsigned char[])
                { 5, 224, 238, 168, 213, 155, 140, 95, 208, 200, 219, 162, 95, 201, 94, 65 },
            .route_src_plen_val = 96,
            .route_prefix_val = (unsigned char[])
                { 225, 33, 114, 8, 246, 83, 140, 92, 194, 195, 254, 241, 86, 75, 18, 40 },
            .route_plen_val = 128,
            .expected_rc = -24
        },
        {
            .prefix_val = (unsigned char[])
                { 204, 191, 204, 17, 179, 148, 97, 201, 24, 33, 133, 32, 138, 138, 104, 235 },
            .plen_val = 128,
            .src_prefix_val = (unsigned char[])
                { 167, 145, 127, 130, 201, 185, 216, 226, 87, 1, 78, 203, 236, 64, 33, 184 },
            .src_plen_val = 0,
            .route_src_prefix_val = (unsigned char[])
                { 0, 237, 201, 179, 130, 42, 124, 154, 75, 1, 186, 213, 139, 34, 192, 50 },
            .route_src_plen_val = 96,
            .route_prefix_val = (unsigned char[])
                { 180, 64, 181, 125, 249, 141, 95, 81, 142, 173, 28, 122, 238, 61, 50, 238 },
            .route_plen_val = 128,
            .expected_rc = 1
        },
        {

            .prefix_val = (unsigned char[])
                { 201, 5, 52, 158, 160, 192, 253, 113, 137, 217, 19, 232, 162, 114, 41, 141 },
            .plen_val = 128,
            .src_prefix_val = (unsigned char[])
                { 234, 209, 73, 225, 36, 213, 61, 230, 152, 59, 215, 238, 134, 233, 23, 140 },
            .src_plen_val = 96,
            .route_src_prefix_val = (unsigned char[])
                { 5, 224, 238, 168, 213, 155, 140, 95, 208, 200, 219, 162, 95, 201, 94, 65 },
            .route_src_plen_val = 0,
            .route_prefix_val = (unsigned char[])
                { 225, 33, 114, 8, 246, 83, 140, 92, 194, 195, 254, 241, 86, 75, 18, 40 },
            .route_plen_val = 128,
            .expected_rc = -1
        },
        // TODO: Bug?
        {
            .prefix_val = (unsigned char[])
                { 204, 191, 204, 17, 179, 148, 97, 201, 24, 33, 133, 32, 138, 138, 104, 235 },
            .plen_val = 128,
            .src_prefix_val = (unsigned char[])
                { 167, 145, 127, 130, 201, 185, 216, 226, 87, 1, 78, 203, 236, 64, 33, 184 },
            .src_plen_val = 96,
            .route_src_prefix_val = (unsigned char[])
                { 5, 224, 238, 168, 213, 155, 140, 95, 208, 200, 219, 162, 95, 201, 94, 65 },
            .route_src_plen_val = 96,
            .route_prefix_val = (unsigned char[])
                { 204, 191, 204, 17, 179, 148, 97, 201, 24, 33, 133, 32, 138, 138, 104, 235 },
            .route_plen_val = 10,
            .expected_rc = -1
        },
        {

            .prefix_val = (unsigned char[])
                { 201, 5, 52, 158, 160, 192, 253, 113, 137, 217, 19, 232, 162, 114, 41, 141 },
            .plen_val = 10,
            .src_prefix_val = (unsigned char[])
                { 234, 209, 73, 225, 36, 213, 61, 230, 152, 59, 215, 238, 134, 233, 23, 140 },
            .src_plen_val = 96,
            .route_src_prefix_val = (unsigned char[])
                { 5, 224, 238, 168, 213, 155, 140, 95, 208, 200, 219, 162, 95, 201, 94, 65 },
            .route_src_plen_val = 96,
            .route_prefix_val = (unsigned char[])
                { 201, 5, 52, 158, 160, 192, 253, 113, 137, 217, 19, 232, 162, 114, 41, 141 },
            .route_plen_val = 128,
            .expected_rc = 1
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
            fprintf(stderr, "prefix: %s\n", str_of_array(prefix, 16));
            fprintf(stderr, "plen: %d\n", plen);
            fprintf(stderr, "src_prefix: %s\n", str_of_array(src_prefix, 16));
            fprintf(stderr, "src_plen: %d\n", src_plen);
            fprintf(stderr, "route->src->prefix: %s\n", str_of_array(route.src->prefix, 16));
            fprintf(stderr, "route->src->plen: %d\n", route.src->plen);
            fprintf(stderr, "route->src->src_prefix: %s\n", str_of_array(route.src->src_prefix, 16));
            fprintf(stderr, "route->src->src_plen: %d\n", route.src->src_plen);
            fprintf(stderr, "expected rc: %d\n", tcs[i].expected_rc);
            fprintf(stderr, "computed rc: %d\n", rc);
            fflush(stderr);
        }
    }
    free(route.src);
}

void find_route_slot_test(void) {
    int i, num_of_cases, rc, new_return;
    unsigned char *prefix, *src_prefix;
    unsigned char plen, src_plen;

    typedef struct test_case {
        unsigned char *prefix_val;
        unsigned char plen_val;
        unsigned char *src_prefix_val;
        unsigned char src_plen_val;
        int expected_rc;
        int expected_new_return;
    } test_case;

    test_case tcs[] =
    {
        {
            .prefix_val = (unsigned char[])
                { 145, 103, 214, 219, 183, 36, 182, 66, 11, 175, 199, 131, 227, 198, 7, 136 },
            .plen_val = 54,
            .src_prefix_val = (unsigned char[])
                { 97, 114, 138, 89, 89, 22, 41, 71, 180, 179, 225, 48, 49, 80, 170, 194 },
            .src_plen_val = 99,
            .expected_rc = 0,
            .expected_new_return = -1
        },
        {

            .prefix_val = (unsigned char[])
                { 183, 128, 157, 111, 164, 84, 177, 176, 3, 120, 51, 230, 62, 58, 111, 159 },
            .plen_val = 24,
            .src_prefix_val = (unsigned char[])
                { 172, 249, 249, 5, 15, 34, 76, 195, 213, 45, 244, 6, 125, 158, 200, 53 },
            .src_plen_val = 90,
            .expected_rc = 0,
            .expected_new_return = -1
        },
    };

    num_of_cases = sizeof(tcs) / sizeof(test_case);
    for(i = 0; i < num_of_cases; ++i) {
        prefix = tcs[i].prefix_val;
        plen = tcs[i].plen_val;
        src_prefix = tcs[i].src_prefix_val;
        src_plen = tcs[i].src_plen_val;

        rc = find_route_slot(prefix, plen, src_prefix, src_plen, &new_return);
        printf("rc: %d\n", rc);
        printf("new_return: %d\n", rc == -1 ? new_return : -1 );
        /* if (!babel_check((rc == -1 && new_return == tcs[i].expected_new_return) || rc == tcs[i].expected_rc)) { */
        /*     fprintf(stderr, "Failed test on route_compare\n"); */
        /*     fprintf(stderr, "prefix: %s\n", str_of_array(prefix, plen)); */
        /*     fprintf(stderr, "plen: %d\n", plen); */
        /*     fprintf(stderr, "src_prefix: %s\n", str_of_array(src_prefix, src_plen)); */
        /*     fprintf(stderr, "src_plen: %d\n", src_plen); */
        /*     fprintf(stderr, "expected rc: %d\n", tcs[i].expected_rc); */
        /*     fprintf(stderr, "computed rc: %d\n", rc); */
        /*     fflush(stderr); */
        /* } */
    }
}

void route_setup(void) {
    int i;
    // Randomly generated
    unsigned char prefixes[6][16] =
      {
        { 160, 175, 139, 76, 149, 129, 138, 109, 209, 43, 127, 92, 8, 202, 53, 182 },
        { 118, 204, 77, 156, 52, 93, 35, 51, 137, 29, 164, 158, 179, 101, 255, 252 },
        { 227, 216, 75, 160, 38, 254, 131, 189, 88, 42, 56, 139, 244, 255, 11, 82 },
        { 78, 162, 240, 49, 189, 24, 46, 203, 201, 107, 41, 160, 213, 182, 197, 23 },
        { 69, 198, 228, 78, 253, 128, 30, 115, 115, 189, 34, 209, 203, 126, 38, 62 },
        { 93, 135, 206, 145, 214, 232, 94, 9, 247, 22, 71, 251, 157, 3, 77, 167 }
      };
    int plens[] = {77, 101, 105, 12, 40, 25};
    unsigned char src_prefixes[6][16] =
      {
        { 157, 105, 76, 111, 96, 98, 35, 253, 235, 49, 69, 120, 108, 140, 34, 198 },
        { 153, 214, 219, 41, 222, 82, 207, 131, 155, 79, 202, 239, 25, 208, 233, 179 },
        { 24, 27, 163, 100, 57, 21, 220, 196, 63, 155, 246, 218, 80, 49, 160, 174 },
        { 26, 137, 255, 238, 199, 6, 224, 128, 87, 142, 8, 197, 49, 142, 106, 113 },
        { 173, 76, 71, 184, 21, 200, 70, 185, 15, 19, 223, 62, 165, 179, 210, 92 },
        { 107, 103, 113, 193, 138, 153, 175, 32, 159, 28, 70, 247, 160, 25, 204, 190 },
      };
    int src_plens[] = {100, 115, 96, 50, 37, 81};
    routes = malloc(6 * sizeof(struct babel_route*));
    for(i = 0; i < 6; i++) {
        routes[i] = malloc(sizeof(struct babel_route));
        routes[i]->src = malloc(sizeof(struct source));
        routes[i]->src->plen = plens[i];
        routes[i]->src->src_plen = src_plens[i];
        memcpy(routes[i]->src->prefix, prefixes[i], 16);
        memcpy(routes[i]->src->src_prefix, src_prefixes[i], 16);
    }

}

void route_tear_down(void) {
    int i;
    for(i = 0; i < 6; i++) {
        free(routes[i]->src);
        free(routes[i]);
    }
    free(routes);
}

void route_test_suite() {
    route_setup();
    run_test(route_compare_test, "route_compare_test");
    run_test(find_route_slot_test, "find_route_slot");
    route_tear_down();
}