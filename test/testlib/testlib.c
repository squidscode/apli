#include <stdlib.h>
#include <stdio.h>

#ifdef COLORED_TESTS

#define GREEN "\x1b[42;1m"
#define RED   "\x1b[41;1m"
#define FRED  "\x1b[31;1m"
#define BLUE  "\x1b[44;1m"
#define RESET "\x1b[0m"

#define init_test() \
    int test_counter = 1
#define assertTrueQuiet(expr) \
    if(!(expr)) { \
        printf(RED "[FAIL]" RESET " Test #%d\n", test_counter); \
        printf("  " FRED "- Line #%d: %s " RESET "\n", __LINE__, #expr); \
    } \
    test_counter++;
#define assertTrue(expr) \
    if((expr)) { \
        printf(GREEN "[PASS]" RESET " Test #%d\n", test_counter); \
    } else { \
        printf(RED "[FAIL]" RESET " Test #%d\n", test_counter); \
        printf("  " FRED "- Line #%d: %s" RESET "\n", __LINE__, #expr); \
    } \
    test_counter++;
#define assertFalseQuiet(expr)  assertTrueQuiet(!(expr))
#define assertFalse(expr)       assertTrue(!(expr))

#else
#define init_test() \
    int test_counter = 1
#define assertTrueQuiet(expr) \
    if(!(expr)) { \
        printf("[FAIL] TEST #%d\n", test_counter); \
        printf("  - LINE #%d: %s\n", __LINE__, #expr); \
    } \
    test_counter++;
#define assertTrue(expr) \
    if((expr)) { \
        printf("[PASS] Test #%d\n", test_counter); \
    } else { \
        printf("[FAIL] Test #%d\n", test_counter); \
        printf("  - Line #%d: %s\n", __LINE__, #expr); \
    } \
    test_counter++;
#define assertFalseQuiet(expr)  assertTrueQuiet(!(expr))
#define assertFalse(expr)       assertTrue(!(expr))
#endif

init_test();