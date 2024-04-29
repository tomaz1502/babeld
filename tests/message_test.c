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


#include <netinet/in.h>

#include "../babeld.h"
#include "../util.h"
#include "../net.h"
#include "../interface.h"
#include "../source.h"
#include "../neighbour.h"
#include "../route.h"
#include "../kernel.h"
#include "../xroute.h"
#include "../resend.h"

#include "../message.h"

#include "minunit.h"

MU_TEST(trivial)
{
	mu_check(1);
}

MU_TEST_SUITE(message_test_suite)
{
	MU_RUN_TEST(trivial);
}

int
run_message_tests(void) {
  printf("--------------------------------------------\n");
  printf("Running tests for message.c.\n");
  printf("--------------------------------------------\n");
  MU_RUN_SUITE(message_test_suite);
  MU_REPORT();
  return MU_EXIT_CODE;
}
