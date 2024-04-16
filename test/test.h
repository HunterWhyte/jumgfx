#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "assert.h"

// Text formatting macros
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define NO_UNDERLINE "\033[24m"

// Foreground color macros
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

typedef int (*test_function)(void);

typedef struct test {
  char* name;
  test_function func;
} test_t;

#define MAX_TESTS 1024
typedef struct test_suite {
  char* name;
  test_t tests[MAX_TESTS];
  int num_tests;
} suite_t;

#define TEST(suite, testname)                                         \
  int testname();                                                     \
  __attribute__((constructor)) void register_##suite##_##testname() { \
    static test_t t = {.name = #testname, .func = testname};          \
    assert(suite.num_tests < MAX_TESTS);                              \
    suite.tests[suite.num_tests] = t;                                 \
    suite.num_tests++;                                                \
  }                                                                   \
  int testname()

#define SUITE(suitename) suite_t suitename = {.name = #suitename}

static inline int run_test(test_t t) {
  printf(BOLD MAGENTA "========= TEST %s ==========\n" RESET, t.name);
  if (t.func()) {
    printf(RED BOLD "TEST %s " UNDERLINE "FAILED\n" RESET, t.name);
    return 1;
  } else {
    printf(GREEN BOLD "SUCCESS\n" RESET);
    return 0;
  }
}

static inline int run_suite(suite_t suite) {
  int failures = 0;
  printf(CYAN "==================================================\n" RESET);
  printf(BOLD MAGENTA "\tRUNING SUITE %s \n" RESET, suite.name);
  printf(CYAN "==================================================\n" RESET);

  for (int i = 0; i < suite.num_tests; i++) {
    failures += run_test(suite.tests[i]);
  }
  printf("==================================================\n");

  if (failures == 0) {
    printf(GREEN BOLD "%s ALL TESTS (%d) PASSED\n" RESET, suite.name, suite.num_tests);
  } else {
    printf(RED BOLD "%s SUITE FAILURES %d/%d\n" RESET, suite.name, failures, suite.num_tests);
  }
  printf("==================================================\n");
}

#define ASSERT(condition)                                        \
  if (!(condition)) {                                            \
    printf(RED BOLD "!!!! ASSERTION FAILED !!!!" RESET BOLD      \
                    "\n\tcondition:\n\t%s\n\tTEST: %s\n\tLINE: " \
                    "%d\n" RESET,                                \
           #condition, __func__, __LINE__);                      \
    return 1;                                                    \
  }

#define ASSERT_EQ(a, b)                                                            \
  if (a != b) {                                                                    \
    printf(RED BOLD "!!!! ASSERTION FAILED !!!!" RESET BOLD                        \
                    "\n\tequality:\n\t%s != %s\n\t%X != %X \n\tTEST: %s\n\tLINE: " \
                    "%d\n" RESET,                                                  \
           #a, #b, a, b, __func__, __LINE__);                                      \
    return 1;                                                                      \
  }

#define EPSILON 0.000001
#define ASSERT_FLOAT_EQ(a, b)                                                            \
  if (!(((-EPSILON) < (a - b)) && ((a - b) < (EPSILON)))) {                              \
    printf(RED BOLD "!!!! ASSERTION FAILED !!!!" RESET BOLD                              \
                    "\n\tfloat equality:\n\t%s != %s\n\t%f != %f \n\tTEST: %s\n\tLINE: " \
                    "%d\n" RESET,                                                        \
           #a, #b, a, b, __func__, __LINE__);                                            \
    return 1;                                                                            \
  }

#define ASSERT_STRING_EQ(a, b)                                                            \
  if (strcmp(a, b) != 0) {                                                                \
    printf(RED BOLD "!!!! ASSERTION FAILED !!!!" RESET BOLD                               \
                    "\n\tstring equality:\n\t%s != %s\n\t%s != %s \n\tTEST: %s\n\tLINE: " \
                    "%d\n" RESET,                                                         \
           #a, #b, a, b, __func__, __LINE__);                                             \
    return 1;                                                                             \
  }

#ifdef __cplusplus
}
#endif