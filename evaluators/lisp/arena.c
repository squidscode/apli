#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

size_t bytes_allocated = 0;
_Atomic size_t beginning_of_block = 0;

// 4 gigabytes of memory.
#define MAX_SIZE 4000000000
// #define MAX_SIZE 4000000

void *_malloc(size_t sz) {
    beginning_of_block += sz;
    // bytes_allocated += sz;
    return (void*) (beginning_of_block - sz);
}

void _initialize_memory() {
    beginning_of_block = (size_t) malloc(MAX_SIZE);
}

#ifdef ARENA_ALLOCATOR
#define malloc(sz)  (_malloc(sz))
#define free(ptr)
#endif