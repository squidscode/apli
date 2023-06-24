#include <stdlib.h>
#include <stdio.h>

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
        printf("[PASS] TEST #%d\n", test_counter); \
    } else { \
        printf("[FAIL] TEST #%d\n", test_counter); \
        printf("  - LINE #%d: %s\n", __LINE__, #expr); \
    } \
    test_counter++;
#define assertFalseQuiet(expr)  assertTrueQuiet(!(expr))
#define assertFalse(expr)       assertTrue(!(expr))


init_test();