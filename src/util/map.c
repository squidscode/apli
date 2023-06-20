#ifndef MAP_C
#define MAP_C

#include <stdlib.h>
#include "vector.c"
#include "list.c"

// Resize to 2x when 80% capacity is hit.
#define RESIZE_RATIO 0.80

// NOTE: Value_type must have a list predefined in order for
// this to work.
#define define_map(key_type, value_type) \
    struct _##key_type##_##value_type##_map_; \
    typedef struct _##key_type##_##value_type##_map_fns_ { \
        size_t (*hash)(key_type); \
        void (*insert)(struct _##key_type##_##value_type##_map_*, key_type, value_type); \
        value_type (*at)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*erase)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*count)(struct _##key_type##_##value_type##_map_*, key_type); \
        void (*free)(struct _##key_type##_##value_type##_map_*); \
    } _##key_type##_##value_type##_map_fns_t; \
    typedef struct _##key_type##_##value_type##_map_match_ { \
        size_t hash; \
        value_type val; \
    } _##key_type##_##value_type##_map_match_t; \
    define_list(_##key_type##_##value_type##_map_match_t); \
    typedef List(_##key_type##_##value_type##_map_match_t)* _##key_type##_##value_type##_map_match_list_t; \
    define_vector(_##key_type##_##value_type##_map_match_list_t); \
    typedef struct _##key_type##_##value_type##_map_ { \
        Vector(_##key_type##_##value_type##_map_match_list_t) *buckets; \
        size_t size; \
        _##key_type##_##value_type##_map_fns_t *fns; \
    } _##key_type##_##value_type##_map_t; \
    size_t _default_##key_type##_##value_type##_map_hash_(key_type key) { \
        size_t size = sizeof(key); \
        size_t mod = (sizeof(size_t) / sizeof(char)); \
        size_t offset = 0; \
        void *ptr = (void*) &key; \
        size_t hash = 0; \
        for(size_t i = 0; i < size; ++i) { \
            hash ^= (((char*) ptr++)[i] << (8 * offset++)); \
            offset %= mod; \
        } \
        return hash; \
    } \
    void _allocate_into_bucket_##key_type##_##value_type##_map_(_##key_type##_##value_type##_map_t *map, \
        size_t key_hash, value_type val) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        if(bucket == NULL) { \
            bucket = new_list(_##key_type##_##value_type##_map_match_t); \
        } \
        _##key_type##_##value_type##_map_match_t match = {key_hash, val}; \
        list_push_back(bucket, match); \
    } \
    void _check_resize_##value_type##_##key_type##_map_(_##key_type##_##value_type##_map_t *map) { \
        size_t buckets_size = vector_size(map->buckets); \
        if (map->size >= RESIZE_RATIO * buckets_size) { \
            size_t new_cap = 2 * buckets_size; \
            vector_resize_val(map->buckets, new_cap, NULL); \
            for(size_t i = 0; i < buckets_size; ++i) { \
                size_t mask = (new_cap - 1); \
                _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, i); \
                if(bucket != NULL) { \
                    size_t list_size = list_size(bucket); \
                    for(size_t ind = 0; ind < list_size(bucket); ++ind) { \
                        _##key_type##_##value_type##_map_match_t match = list_get_first(bucket); \
                        list_pop_front(bucket); \
                        _allocate_into_bucket_##key_type##_##value_type##_map_(map, match.hash, match.val); \
                    } \
                } \
            } \
        } \
    } \
    void _##key_type##_##value_type##_map_insert_(_##key_type##_##value_type##_map_t *map, key_type key, \
        value_type val) { \
        _allocate_into_bucket_##key_type##_##value_type##_map_( \
            map, \
            _default_##key_type##_##value_type##_map_hash_(key), \
            val);\
        _check_resize_##value_type##_##key_type##_map_(map); \
    } \
    value_type _##key_type##_##value_type##_map_at_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = _default_##key_type##_##value_type##_map_hash_(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        while(iter_has_next(iter)) { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash) { \
                return match.val; \
            } \
            iter = iter_next(iter); \
        } \
        exit(1); \
    } \
    void _##key_type##_##value_type##_map_erase_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = _default_##key_type##_##value_type##_map_hash_(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        while(iter_has_next(iter)) { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash) {iter_remove(iter, bucket); return;}; \
            iter = iter_next(iter); \
        } \
        exit(1); \
    } \
    size_t _##key_type##_##value_type##_map_count_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = _default_##key_type##_##value_type##_map_hash_(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        if(bucket == NULL) return 0; \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        while(iter_has_next(iter)) { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash) return 1; \
            iter = iter_next(iter); \
        } \
        return 0; \
    } \
    void _##key_type##_##value_type##_map_free_(_##key_type##_##value_type##_map_t *map) { \
        size_t buckets_size = vector_size(map->buckets); \
        for(size_t ind = 0; ind < buckets_size; ++ind) { \
            _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, ind); \
            if(bucket != NULL) free_list(bucket); \
        } \
        vector_free(map->buckets); \
        free(map); \
    } \
    


#endif

// define_list(int);
// typedef int_list_t* int_listp_t;
// define_vector(int_listp_t);
define_map(int,char);

int main() {

}