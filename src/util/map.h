#ifndef MAP_C
#define MAP_C

#include <stdlib.h>
#include "vector.h"
#include "list.h"

// Macros
#define RESIZE_RATIO    0.80
#define INVKEY          1

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

// TODO define a default_key_equals function that checks all bytes in both keys to determine
// if equal
// TODO map_size function

#define define_map(key_type, value_type) \
    struct _##key_type##_##value_type##_map_; \
    /* Virtual function table for dynamic dispatch (polymorphism) */ \
    typedef struct _##key_type##_##value_type##_map_fns_ { \
        void (*insert)(struct _##key_type##_##value_type##_map_*, key_type, value_type); \
        value_type (*at)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*erase)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*count)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*size)(struct _##key_type##_##value_type##_map_*); \
        void (*free)(struct _##key_type##_##value_type##_map_*); \
    } _##key_type##_##value_type##_map_fns_t; \
    \
    /* A `map_match` holds information about a single (key, value) pair. */ \
    typedef struct _##key_type##_##value_type##_map_match_ { \
        size_t hash; \
        key_type key; \
        value_type val; \
    } _##key_type##_##value_type##_map_match_t; \
    /* List+Vector definitions for buckets of `map_match`s. */ \
    define_list(_##key_type##_##value_type##_map_match_t); \
    typedef List(_##key_type##_##value_type##_map_match_t)* _##key_type##_##value_type##_map_match_list_t; \
    define_vector(_##key_type##_##value_type##_map_match_list_t); \
    \
    /* A `map` contains a hash function, buckets of `map_match`, the size of the map, */ \
    typedef struct _##key_type##_##value_type##_map_ { \
        size_t (*hash)(key_type); \
        size_t (*key_eq)(key_type, key_type); \
        Vector(_##key_type##_##value_type##_map_match_list_t) *buckets; \
        size_t size; \
        _##key_type##_##value_type##_map_fns_t *fns; \
    } _##key_type##_##value_type##_map_t; \
    \
    /* The default hash function for the map. */ \
    size_t _default_##key_type##_##value_type##_map_hash_(key_type key) { \
        size_t size = (sizeof(key_type) / sizeof(char)); \
        size_t mod = (sizeof(size_t) / sizeof(char)); \
        size_t offset = 0; \
        char *ptr = (char*) ((void*) &key); \
        size_t hash = 0; \
        for(size_t i = 0; i < size; ++i) { \
            hash ^= ((255UL & ptr[i]) << (8 * offset++)); \
            offset %= mod; \
        } \
        return hash; \
    } \
    \
    /* The default key_equals function. */ \
    size_t _default_##key_type##_##value_type##_key_eq_(key_type key1, key_type key2) { \
        size_t size = (sizeof(key_type) / sizeof(char)); \
        char *ptr1 = (char*) ((void*) &key1); \
        char *ptr2 = (char*) ((void*) &key2); \
        for(size_t i = 0; i < size; ++i) { \
            if(ptr1[i] != ptr2[i]) return 0; \
        } \
        return 1; \
    } \
    \
    /* Allocates the (hash, key, value) into the appropriate bucket in the map. */ \
    void _allocate_into_bucket_##key_type##_##value_type##_map_(_##key_type##_##value_type##_map_t *map, \
        size_t key_hash, key_type key, value_type val) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        if(bucket == NULL) { \
            bucket = new_list(_##key_type##_##value_type##_map_match_t); \
        } else { \
            map->fns->erase(map, key); \
        } \
        _##key_type##_##value_type##_map_match_t match = {key_hash, key, val}; \
        list_push_back(bucket, match); \
        vector_set(map->buckets, (key_hash & mask), bucket); \
    } \
    \
    /* Check AND resize the map if the size of the map exceeds RESIZE_RATIO. */ \
    void _check_resize_##value_type##_##key_type##_map_(_##key_type##_##value_type##_map_t *map) { \
        size_t buckets_size = vector_size(map->buckets); \
        if (map->size < RESIZE_RATIO * buckets_size) return; \
        size_t new_cap = (buckets_size << 1); \
        vector_resize_val(map->buckets, new_cap, NULL); \
        for(size_t i = 0; i < buckets_size; ++i) { \
            size_t mask = (new_cap - 1); \
            _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, i); \
            if(bucket == NULL) continue; \
            size_t list_size = list_size(bucket); \
            for(size_t ind = 0; ind < list_size; ++ind) { \
                _##key_type##_##value_type##_map_match_t match = list_get_first(bucket); \
                list_pop_front(bucket); \
                _allocate_into_bucket_##key_type##_##value_type##_map_(map, match.hash, match.key, match.val); \
            } \
        } \
    } \
    \
    void _##key_type##_##value_type##_map_insert_(_##key_type##_##value_type##_map_t *map, key_type key, \
        value_type val) { \
        _allocate_into_bucket_##key_type##_##value_type##_map_( \
            map, \
            map->hash(key), \
            key, \
            val);\
        map->size += 1; \
        _check_resize_##value_type##_##key_type##_map_(map); \
    } \
    \
    value_type _##key_type##_##value_type##_map_at_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = map->hash(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        do { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash && map->key_eq(match.key, key)) { \
                return match.val; \
            } \
        } while(iter_has_next(iter) && (iter = iter_next(iter), 1)); \
        fprintf(stderr, "INVALID KEY SUPPLIED\n"); \
        exit(INVKEY); \
    } \
    \
    size_t _##key_type##_##value_type##_map_erase_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = map->hash(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        if(bucket == NULL || list_size(bucket) == 0) return 0; \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        do { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash && map->key_eq(match.key, key)) { \
                iter_remove(iter, bucket); \
                map->size -= 1; \
                return 1; \
            }; \
        } while(iter_has_next(iter) && (iter = iter_next(iter), 1)); \
        return 0; \
    } \
    \
    size_t _##key_type##_##value_type##_map_count_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = map->hash(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        if(bucket == NULL || list_size(bucket) == 0) return 0; \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        do { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash && map->key_eq(match.key, key)) return 1; \
        } while(iter_has_next(iter) && (iter = iter_next(iter), 1)); \
        return 0; \
    } \
    \
    size_t _##key_type##_##value_type##_map_size_(_##key_type##_##value_type##_map_t *map) { \
        return map->size; \
    } \
    \
    void _##key_type##_##value_type##_map_free_(_##key_type##_##value_type##_map_t *map) { \
        size_t buckets_size = vector_size(map->buckets); \
        for(size_t ind = 0; ind < buckets_size; ++ind) { \
            _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, ind); \
            if(bucket != NULL) free_list(bucket); \
        } \
        vector_free(map->buckets); \
        free(map); \
    } \
    \
    _##key_type##_##value_type##_map_fns_t _##key_type##_##value_type##_map_v_table_ = { \
        &_##key_type##_##value_type##_map_insert_, \
        &_##key_type##_##value_type##_map_at_, \
        &_##key_type##_##value_type##_map_erase_, \
        &_##key_type##_##value_type##_map_count_, \
        &_##key_type##_##value_type##_map_size_, \
        &_##key_type##_##value_type##_map_free_, \
    }; \
    \
    _##key_type##_##value_type##_map_t* _##key_type##_##value_type##_new_map() { \
        _##key_type##_##value_type##_map_t *map = (_##key_type##_##value_type##_map_t*) malloc(sizeof(_##key_type##_##value_type##_map_t)); \
        map->buckets = new_vector(_##key_type##_##value_type##_map_match_list_t); \
        vector_push_back(map->buckets, NULL); \
        map->size = 0; \
        map->fns = &_##key_type##_##value_type##_map_v_table_; \
        map->hash = &_default_##key_type##_##value_type##_map_hash_; \
        map->key_eq = &_default_##key_type##_##value_type##_key_eq_; \
        return map; \
    }

#ifndef UNTYPED_MAP_FN
#define UNTYPED_MAP_FN
#define Map(key_type, value_type)                   _##key_type##_##value_type##_map_t
#define new_map(key_type, value_type)               (_##key_type##_##value_type##_new_map())
#define set_map_key_eq(map, fn_ref)                 ((map)->key_eq = (fn_ref))
#define set_map_hash(map, fn_ref)                   ((map)->hash = (fn_ref))
#define map_insert(map, key, value)                 ((map)->fns->insert((map), (key), (value)))
#define map_at(map, key)                            ((map)->fns->at((map), (key)))
#define map_erase(map, key)                         ((map)->fns->erase((map), (key)))
#define map_count(map, key)                         ((map)->fns->count((map), (key)))
#define map_size(map)                               ((map)->fns->size((map)))
#define free_map(map)                               ((map)->fns->free((map)))
#endif

#endif