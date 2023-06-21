#ifndef MAP_C
#define MAP_C

#include <stdlib.h>
#include "vector.h"
#include "list.h"

// Macros
#define RESIZE_RATIO    0.80
#define INVKEY          4

#define _CC(a,b,c)      _ ## a ## _ ## b ## c
#define __CC(...)       _CC(__VA_ARGS__)
#define _KV             key_type, value_type

/*

Defines a map with the given key_type and value_type. 

----- Usage -----
Map(T1, T2) *map = new_map(T1, T2);
  map_insert(map, key, value)       -> void
  map_at(map, key)                  -> value
  map_erase(map, key)               -> size_t
  map_count(map, key)               -> size_t
  free_map(map)                     -> void

*/
#define define_map(key_type, value_type) \
    struct _##key_type##_##value_type##_map_; \
    typedef struct _##key_type##_##value_type##_map_fns_ { \
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
        size_t (*hash)(key_type); \
        Vector(_##key_type##_##value_type##_map_match_list_t) *buckets; \
        size_t size; \
        _##key_type##_##value_type##_map_fns_t *fns; \
    } _##key_type##_##value_type##_map_t; \
    size_t _default_##key_type##_##value_type##_map_hash_(key_type key) { \
        size_t size = (sizeof(key_type) / sizeof(char)); \
        size_t mod = (sizeof(size_t) / sizeof(char)); \
        size_t offset = 0; \
        char *ptr = (char*) ((void*) &key); \
        size_t hash = 0; \
        for(size_t i = 0; i < size; ++i) { \
            /* printf("Offset %lu, ptr[%lu] = %ul\n", offset, i, (255UL & ptr[i])); */ \
            hash ^= ((255UL & ptr[i]) << (8 * offset++)); \
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
        /* printf("PLACE (%lu, %c) into %zu\n", match.hash, match.val, key_hash & mask); */ \
        vector_set(map->buckets, (key_hash & mask), bucket); \
    } \
    void _check_resize_##value_type##_##key_type##_map_(_##key_type##_##value_type##_map_t *map) { \
        size_t buckets_size = vector_size(map->buckets); \
        if (map->size >= RESIZE_RATIO * buckets_size) { \
            size_t new_cap = (buckets_size << 1); \
            /* printf("NEW BUCKET SIZE: %lu\n", new_cap); */ \
            vector_resize_val(map->buckets, new_cap, NULL); \
            for(size_t i = 0; i < buckets_size; ++i) { \
                size_t mask = (new_cap - 1); \
                _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, i); \
                if(bucket != NULL) { \
                    size_t list_size = list_size(bucket); \
                    for(size_t ind = 0; ind < list_size; ++ind) { \
                        _##key_type##_##value_type##_map_match_t match = list_get_first(bucket); \
                        /* printf("REALLOCATING (%lu, %c)\n", match.hash, match.val); */ \
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
            map->hash(key), \
            val);\
        map->size += 1; \
        _check_resize_##value_type##_##key_type##_map_(map); \
    } \
    value_type _##key_type##_##value_type##_map_at_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = map->hash(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        do { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash) { \
                return match.val; \
            } \
        } while(iter_has_next(iter) && (iter = iter_next(iter), 1)); \
        fprintf(stderr, "INVALID KEY SUPPLIED\n"); \
        exit(INVKEY); \
    } \
    size_t _##key_type##_##value_type##_map_erase_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = map->hash(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        if(bucket == NULL || list_size(bucket) == 0) return 0; \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        do { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash) {iter_remove(iter, bucket); map->size -= 1; return 1;}; \
        } while(iter_has_next(iter) && (iter = iter_next(iter), 1)); \
        return 0; \
    } \
    size_t _##key_type##_##value_type##_map_count_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = map->hash(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        /* printf("COUNT looking for hash [%lu] in bucket [%lu]\n", key_hash, (key_hash & mask)); */ \
        if(bucket == NULL || list_size(bucket) == 0) return 0; \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        do { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash) return 1; \
        } while(iter_has_next(iter) && (iter = iter_next(iter), 1)); \
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
    _##key_type##_##value_type##_map_fns_t _##key_type##_##value_type##_map_v_table_ = { \
        &_##key_type##_##value_type##_map_insert_, \
        &_##key_type##_##value_type##_map_at_, \
        &_##key_type##_##value_type##_map_erase_, \
        &_##key_type##_##value_type##_map_count_, \
        &_##key_type##_##value_type##_map_free_, \
    }; \
    _##key_type##_##value_type##_map_t* _##key_type##_##value_type##_new_map() { \
        _##key_type##_##value_type##_map_t *map = (_##key_type##_##value_type##_map_t*) malloc(sizeof(_##key_type##_##value_type##_map_t)); \
        map->buckets = new_vector(_##key_type##_##value_type##_map_match_list_t); \
        vector_push_back(map->buckets, NULL); \
        map->size = 0; \
        map->fns = &_##key_type##_##value_type##_map_v_table_; \
        map->hash = &_default_##key_type##_##value_type##_map_hash_; \
        return map; \
    }

#ifndef UNTYPED_MAP_FN
#define UNTYPED_MAP_FN
#define Map(key_type, value_type)                   _##key_type##_##value_type##_map_t
#define new_map(key_type, value_type)               (_##key_type##_##value_type##_new_map())
#define set_map_hash(map, fn_ref)                   ((map)->hash = (fn))
#define map_insert(map, key, value)                 ((map)->fns->insert((map), (key), (value)))
#define map_at(map, key)                            ((map)->fns->at((map), (key)))
#define map_erase(map, key)                         ((map)->fns->erase((map), (key)))
#define map_count(map, key)                         ((map)->fns->count((map), (key)))
#define free_map(map)                               ((map)->fns->free((map)))
#endif

#endif