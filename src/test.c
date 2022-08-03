#ifndef TESTING
#define TESTING

#include <stdio.h>
#include <assert.h>

#ifdef VERBOSE
#define log(...)            printf(__VA_ARGS__)
#else // VERBOSE
#define log_rec4(a, ...)    (void) (a) __VA_OPT__(; log_rec(__VA_ARGS__))
#define log_rec3(a, ...)    (void) (a) __VA_OPT__(; log_rec4(__VA_ARGS__))
#define log_rec2(a, ...)    (void) (a) __VA_OPT__(; log_rec3(__VA_ARGS__))
#define log_rec1(a, ...)    (void) (a) __VA_OPT__(; log_rec2(__VA_ARGS__))
#define log_rec(a, ...)     (void) (a) __VA_OPT__(; log_rec1(__VA_ARGS__))
// #define log(...)            log_rec(__VA_ARGS__)
#define log(...)            do {if(0)printf(__VA_ARGS__);} while(0)
#endif // VERBOSE

#define ANY_SECT(pre, name, suf)    log("%s%s%s", pre, name, suf)
#define SECTION(name)       ANY_SECT("\n==== ", name, " ====\n\n")
#define SECTIONn(name)      ANY_SECT("\n==== ", name, " ====\n")
#define SUBSECTION(name)    ANY_SECT("\n--- ", name, " ---\n\n")
#define nSUBSECTION(name)   ANY_SECT("--- ", name, " ---\n\n")

#define DOUBLE_EPSILON  1e-32

u64 failed = 0;

//Funcao para mostrar os resultadaos do teste 
void end_tests(const char *test_name) {
    log("\n");
    printf("%s: ", test_name);
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

//Contar as falhas
void err() {
    failed += 1;
}

//Funcoes testar tipos especificos de variaveis
//Checando o valor esperado

//////////  u32  ///////

void u32_err_equal(const char *test_name,
        const u32 expected, const u32 actual) {
    err();
    printf("\nFALIED TEST: %s:\n"
        "    expected(%u) equal actual(%u)\n",
            test_name, expected, actual);
}

b32 u32_expect_equal(const char *test_name,
        const u32 expected, const u32 actual) {
    if ( expected != actual ) {
        u32_err_equal(test_name, expected, actual);
        return 0;
    }
    return 1;
}

void u32_err_greater_eq(const char *test_name,
        const u32 expected_upper, const u32 actual) {
    err();
    printf("\nFALIED TEST: %s:\n"
        "    expected(%u) ge_eq than actual(%u)\n",
            test_name, expected_upper, actual);
}

b32 u32_expect_greater_eq(const char *test_name,
        const u32 expected_upper, const u32 actual) {
    if ( expected_upper < actual ) {
        u32_err_greater_eq(test_name, expected_upper, actual);
        return 0;
    }
    return 1;
}

void u32_err_between(const char *test_name,
        const u32 lower, const u32 upper, const u32 actual) {
    err();
    printf("\nFALIED TEST: %s:\n"
        "    expected range[%u, %u], actual(%u)\n",
            test_name, lower, upper, actual);
}

b32 u32_expect_between(const char *test_name,
        const u32 lower, const u32 upper, const u32 actual) {
    assert( lower <= upper );
    if ( lower > actual || actual > upper ) {
        u32_err_between(test_name, lower, upper, actual);
        return 0;
    }
    return 1;
}

//////////  f64  ///////

void f64_err_equal(const char *test_name,
        const f64 expected, const f64 actual, const f64 tol) {
    err();
    printf("\nFALIED TEST: %s:\n"
        "    expected(%lg) equal actual(%lg), diff(%lg), tol: %lg\n",
            test_name, expected, actual, expected - actual, tol);
}

b32 f64_expect_equal_tol(const char *test_name,
        const f64 expected, const f64 actual, const f64 tol) {
    assert( tol >= 0 );
    const f64 diff = expected - actual;
    if ( -tol > diff || diff > tol ) {
        f64_err_equal(test_name, expected, actual, tol);
        return 0;
    }
    return 1;
}

b32 f64_expect_equal(const char *test_name,
        const f64 expected, const f64 actual) {
    return f64_expect_equal_tol(test_name,
            expected, actual, DOUBLE_EPSILON);
}

void f64_err_less(const char *test_name,
        const f64 expected_upper, const f64 actual) {
    err();
    printf("\nFALIED TEST: %s:\n"
        "    expected(%lg) less than actual(%lg), diff %lg\n",
            test_name, expected_upper, actual,
            expected_upper - actual);
}

b32 f64_expect_less(const char *test_name,
        const f64 expected_lower, const f64 actual) {
    if ( expected_lower >= actual ) {
        f64_err_less(test_name, expected_lower, actual);
        return 0;
    }
    return 1;
}

void f64_err_less_eq(const char *test_name,
        const f64 expected_lower, const f64 actual) {
    err();
    printf("\nFALIED TEST: %s:\n"
        "    expected(%lg) le_eq than actual(%lg), diff %lg\n",
            test_name, expected_lower, actual,
            expected_lower - actual);
}

b32 f64_expect_less_eq(const char *test_name,
        const f64 expected_lower, const f64 actual) {
    if ( expected_lower > actual ) {
        f64_err_less(test_name, expected_lower, actual);
        return 0;
    }
    return 1;
}

void f64_err_greater_eq(const char *test_name,
        const f64 expected_upper, const f64 actual) {
    err();
    printf("\nFALIED TEST: %s:\n"
        "    expected(%lg) ge_eq than actual(%lg), diff %lg\n",
            test_name, expected_upper, actual,
            expected_upper - actual);
}

b32 f64_expect_greater_eq(const char *test_name,
        const f64 expected_upper, const f64 actual) {
    if ( expected_upper < actual ) {
        f64_err_greater_eq(test_name, expected_upper, actual);
        return 0;
    }
    return 1;
}

#endif // TESTING
