#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../../src/util/map.h"

/**
 * If `COLOR` is defined, then terminal colors are used.
 * If `MEMORY_CHECK` is defined, then a memory leak check is applied.
 * 
 * Usage:
 *   assertTrue(expr)
 *   assertTrueQuiet(expr)
 *   assertFalse(expr)
 *   assertFalseQuiet(expr)
 */

#ifdef COLOR
#define GREEN "\x1b[42;1m"
#define RED   "\x1b[41;1m"
#define FRED  "\x1b[31;1m"
#define BLUE  "\x1b[44;1m"
#define RESET "\x1b[0m"
#else
#define GREEN ""
#define RED   ""
#define FRED  ""
#define BLUE  ""
#define RESET ""
#endif

#ifdef DEBUG
#define dprintf(format, ...)    printf(BLUE "[DEBUG]" RESET " " format, __VA_ARGS__)
#else
#define dprintf(format, ...)
#endif

#define setup_tests() \
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

struct malloc_snapshot {
    size_t counter;
    size_t allocation_size;
    void* allocation_pointer;
};
typedef struct malloc_snapshot malloc_snapshot_t;
typedef void* void_ptr;

define_list(malloc_snapshot_t);
define_map(void_ptr, malloc_snapshot_t);

Map(void_ptr, malloc_snapshot_t) *malloc_history = NULL;
size_t malloc_call_counter = 0;

// New functions:
void* my_malloc(size_t size, const char* file, int line_number) {
    // dprintf("malloc(%lu) called in file %s at line %i\n", size, file, line_number);
    if(malloc_history == NULL) malloc_history = map_new(void_ptr, malloc_snapshot_t);
    void* allocation_pointer = malloc(size);
    malloc_snapshot_t snapshot = {++malloc_call_counter, size, allocation_pointer};
    map_insert(malloc_history, allocation_pointer, snapshot);
    return allocation_pointer;
}

void my_free(void* ptr, const char* file, int line_number) {
    // dprintf("free(%p) called in file %s at line %i\n", ptr, file, line_number);
    if(malloc_history == NULL)
        assert(0 == "Free called before first malloc.");
    if(0 == map_count(malloc_history, ptr))
        assert(0 == "Free called on un-allocated region.");
    map_erase(malloc_history, ptr);
    free(ptr);
}

void print_memory_leak_results() {
    List(_void_ptr_malloc_snapshot_t_map_match_t) *history_list = map_get_list(malloc_history);
    if(0 == list_size(history_list)) {
        printf(GREEN "[MEMORY CHECK]" RESET " All memory was successfully freed.\n");
        return;
    }
    Iterator(_void_ptr_malloc_snapshot_t_map_match_t) *history_iter = list_get_iterator(history_list);
    size_t leaked_bytes_counter = 0;
    while(history_iter != NULL) {
        printf(RED "[MEMORY CHECK]" RESET " #%lu malloc(%lu) @%p was not freed.\n" RESET, 
            iter_val(history_iter).value.counter, iter_val(history_iter).value.allocation_size,
            iter_val(history_iter).value.allocation_pointer);
        leaked_bytes_counter += iter_val(history_iter).value.allocation_size;
        history_iter = iter_next(history_iter);
    }
    printf(RED "[MEMORY CHECK]" RESET " %lu malloc calls were un-freed.\n", map_size(malloc_history));
    printf(RED "[MEMORY CHECK]" RESET " %lu bytes were leaked.\n", leaked_bytes_counter);
    list_free(history_list);
}

#ifdef MEMORY_CHECK
#define malloc(size)        my_malloc(size, __FILE__, __LINE__)
#define free(ptr)           my_free(ptr, __FILE__, __LINE__)
#define teardown_tests()    print_memory_leak_results()
#else
#define teardown_tests()    /* TODO write a default teardown_tests method */
#endif

setup_tests();