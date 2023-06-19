#ifndef MAP_C
#define MAP_C

#include <stdlib.h>
#include "vector.c"
#include "list.c"

// Resize to 2x when 80% capacity is hit.

// NOTE: Value_type must have a list predefined in order for
// this to work.
#define define_map(key_type, value_type) \
    struct _##key_type##_##value_type##_map_; \
    typedef struct _##key_type##_##value_type##_map_fns_ { \
        size_t (*hash)(key_type); \
        void (*insert)(struct _##key_type##_##value_type##_map_*, key_type, value_type); \
        value_type* (*at)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*erase)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*count)(struct _##key_type##_##value_type##_map_*, key_type); \
        void (*free)(struct _##key_type##_##value_type##_map_*); \
    } _##key_type##_##value_type##_map_fns_t; \
    typedef value_type##_list_t* value_type##_listp_t; \
    typedef struct _##key_type##_##value_type##_map_ { \
        Vector(value_type##_listp_t) *buckets; \
        size_t size; \
        _##key_type##_##value_type##_map_fns_t *fns; \
    } _##key_type##_##value_type##_map_t; \
    size_t _default_##key_type##_##value_type##_map_hash_(key_type key) { \
        size_t size = sizeof(key); \
        size_t mod = sizeof(size_t) / sizeof(char); \
        size_t offset = 0; \
        void *ptr = (void*) &key; \
        size_t hash = 0; \
        for(size_t i = 0; i < size; ++i) { \
            hash ^= (((char*) ptr++)[i] << (8 * offset++)); \
            offset %= mod; \
        } \
        return hash; \
    } \
    void _check_resize_##value_type##_##key_type##_map_(_##key_type##_value_type##_map_t* map) { \
        \
    }



#define setup_map(key_type, value_type) \
    define_list(value_type); \
    typedef value_type##_list_t* value_type##_listp_t; \
    define_vector(value_type##_listp_t)


// define_list(int);
// typedef int_list_t* int_listp_t;
// define_vector(int_listp_t);
setup_map(int,char);
define_map(int,char);

#endif