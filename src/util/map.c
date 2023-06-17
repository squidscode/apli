#ifndef MAP_C
#define MAP_C

#include "list.c"

// NOTE: Value_type must have a list predefined in order for
// this to work.
#define define_map(key_type, value_type) \
    typedef struct _##key_type##_##value_type##_bucket_ { \
        \
    } _##key_type##_##value_type##_bucket_t;



define_map(int,int);

#endif