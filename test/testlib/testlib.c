#include <stdlib.h>
#include <stdio.h>

#define init_test() \
    int test_counter = 1
#define assertTrueQuiet(expr) \
    if(!(expr)) { \
        printf("TEST #%d [FAILED]\n", test_counter); \
        printf("  - LINE #%d: %s\n", __LINE__, #expr); \
    } \
    test_counter++;
#define assertTrue(expr) \
    if((expr)) { \
        printf("TEST #%d [PASS]\n", test_counter); \
    } else { \
        printf("TEST #%d [FAILED]\n", test_counter); \
        printf("  - LINE #%d: %s\n", __LINE__, #expr); \
    } \
    test_counter++;

init_test();