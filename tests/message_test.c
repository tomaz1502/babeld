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
