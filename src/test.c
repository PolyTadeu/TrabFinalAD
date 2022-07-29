#ifndef TESTING
#define TESTING

#include <stdio.h>
#include <assert.h>

#ifdef VERBOSE
#define log(...)            printf(__VA_ARGS__)
#else // VERBOSE
#define log(...)
#endif // VERBOSE

#define SECTION(name)       log("\n==== %s ====\n\n", name)

#define DOUBLE_EPSILON  1e-32

u64 failed = 0;

void end_tests() {
    log("\n");
    if ( failed == 1 ) {
        printf("\n======================\n");
        printf("|  %02lu test  failed!  |\n", failed);
        printf("======================\n");
    } else if ( failed > 1 ) {
        printf("\n======================\n");
        printf("|  %02lu tests failed!  |\n", failed);
        printf("======================\n");
    } else {
        printf("Ok!\n");
    }
    failed = 0;
}

void f64_err(const char *test_name, f64 expected, f64 actual, f64 tol) {
    failed += 1;
    printf("\nFALIED TEST: %s:\n"
        "    expected %lg, actual %lg, diff %lg, tol: %lg\n",
            test_name, expected, actual, expected - actual, tol);
}

void f64_expect_equal_tol(const char *test_name, f64 expected, f64 actual, f64 tol) {
    assert( tol >= 0 );
    const f64 diff = expected - actual;
    if ( -tol > diff || diff > tol ) {
        f64_err(test_name, expected, actual, tol);
    }
}

void f64_expect_equal(const char *test_name, f64 expected, f64 actual) {
    f64_expect_equal_tol(test_name, expected, actual, DOUBLE_EPSILON);
}

#endif // TESTING
