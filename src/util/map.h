#ifndef MAP_C
#define MAP_C

#include <stdlib.h>
#include <assert.h>
#include "vector.h"
#include "list.h"

// Macros
#define RESIZE_RATIO    0.80

/**
 *
 * Defines a map with the given key_type and value_type. 
 * 
 * ----- Usage -----
 * Map(T1, T2) *map = map_new(T1, T2);
 *   set_map_hash(map, fn)                  -> void
 *   set_map_key_eq(map, fn)                -> void
 *   map_insert(map, key, value)            -> void
 *   map_at(map, key)                       -> value
 *   map_erase(map, key)                    -> size_t
 *   map_count(map, key)                    -> size_t
 *   map_size(map)                          -> size_t
 *   map_get_list(map)                      -> List(map_match)*
 *   ^^^ Use map_match.key and map_match.value to unpack the match
 *   map_free(map)                          -> void
 */

#define Map(key_type, value_type)                   _##key_type##_##value_type##_map_t
#define MapMatch(key_type, value_type)              _##key_type##_##value_type##_map_match_t
#define map_new(key_type, value_type)               (_##key_type##_##value_type##_new_map())
#define map_set_key_eq(map, fn_ref)                 ((map)->key_eq = (fn_ref))
#define map_set_hash(map, fn_ref)                   ((map)->hash = (fn_ref))
#define map_insert(map, key, value)                 ((map)->fns->insert((map), (key), (value)))
#define map_at(map, key)                            ((map)->fns->at((map), (key)))
#define map_erase(map, key)                         ((map)->fns->erase((map), (key)))
#define map_count(map, key)                         ((map)->fns->count((map), (key)))
#define map_size(map)                               ((map)->fns->size((map)))
#define map_free(map)                               ((map)->fns->destroy((map)))
#define map_get_list(map)                           ((map)->fns->get_list((map)))

#define define_map(key_type, value_type) \
    /* A `map_match` holds information about a single (key, value) pair. */ \
    typedef struct _##key_type##_##value_type##_map_match_ { \
        size_t hash; \
        key_type key; \
        value_type value; \
    } _##key_type##_##value_type##_map_match_t; \
    /* List+Vector definitions for buckets of `map_match`s. */ \
    define_list(_##key_type##_##value_type##_map_match_t); \
    typedef List(_##key_type##_##value_type##_map_match_t)* _##key_type##_##value_type##_map_match_list_t; \
    define_vector(_##key_type##_##value_type##_map_match_list_t); \
    \
    struct _##key_type##_##value_type##_map_; \
    /* Virtual function table for dynamic dispatch (polymorphism) */ \
    typedef struct _##key_type##_##value_type##_map_fns_ { \
        void (*insert)(struct _##key_type##_##value_type##_map_*, key_type, value_type); \
        value_type (*at)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*erase)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*count)(struct _##key_type##_##value_type##_map_*, key_type); \
        size_t (*size)(struct _##key_type##_##value_type##_map_*); \
        List(_##key_type##_##value_type##_map_match_t)* (*get_list)(struct _##key_type##_##value_type##_map_*); \
        void (*destroy)(struct _##key_type##_##value_type##_map_*); \
    } _##key_type##_##value_type##_map_fns_t; \
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
            bucket = list_new(_##key_type##_##value_type##_map_match_t); \
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
            if(bucket == NULL) \
                continue; \
            size_t list_size = list_size(bucket); \
            for(size_t ind = 0; ind < list_size; ++ind) { \
                _##key_type##_##value_type##_map_match_t match = list_get_first(bucket); \
                list_pop_front(bucket); \
                _allocate_into_bucket_##key_type##_##value_type##_map_(map, match.hash, match.key, match.value); \
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
        while(iter != NULL) { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash && map->key_eq(match.key, key)) \
                return match.value; \
            iter = iter_next(iter); \
        } \
        assert(0 == "Invalid key"); \
    } \
    \
    size_t _##key_type##_##value_type##_map_erase_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = map->hash(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        if(bucket == NULL || list_size(bucket) == 0) return 0; \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        while(iter != NULL) { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash && map->key_eq(match.key, key)) { \
                iter_remove(iter, bucket); \
                map->size -= 1; \
                return 1; \
            }; \
            iter = iter_next(iter); \
        } \
        return 0; \
    } \
    \
    size_t _##key_type##_##value_type##_map_count_(_##key_type##_##value_type##_map_t *map, key_type key) { \
        size_t mask = (vector_size(map->buckets) - 1); \
        size_t key_hash = map->hash(key); \
        _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, (key_hash & mask)); \
        if(bucket == NULL || list_size(bucket) == 0) return 0; \
        Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
        while(iter != NULL) { \
            _##key_type##_##value_type##_map_match_t match = iter_val(iter); \
            if(match.hash == key_hash && map->key_eq(match.key, key)) \
                return 1; \
            iter = iter_next(iter); \
        } \
        return 0; \
    } \
    \
    size_t _##key_type##_##value_type##_map_size_(_##key_type##_##value_type##_map_t *map) { \
        return map->size; \
    } \
    \
    List(_##key_type##_##value_type##_map_match_t)* _##key_type##_##value_type##_map_get_list_( \
        _##key_type##_##value_type##_map_t *map) { \
        List(_##key_type##_##value_type##_map_match_t)* matches = list_new(_##key_type##_##value_type##_map_match_t); \
        size_t buckets_size = vector_size(map->buckets); \
        for(size_t ind = 0; ind < buckets_size; ++ind) { \
            _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, ind); \
            if(bucket != NULL) { \
                Iterator(_##key_type##_##value_type##_map_match_t) *iter = get_iterator(bucket); \
                while(iter != NULL) { \
                    list_push_back(matches, iter_val(iter)); \
                    iter = iter_next(iter); \
                } \
            } \
        } \
        return matches; \
    } \
    \
    void _##key_type##_##value_type##_map_free_(_##key_type##_##value_type##_map_t *map) { \
        size_t buckets_size = vector_size(map->buckets); \
        for(size_t ind = 0; ind < buckets_size; ++ind) { \
            _##key_type##_##value_type##_map_match_list_t bucket = vector_get(map->buckets, ind); \
            if(bucket != NULL) list_free(bucket); \
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
        &_##key_type##_##value_type##_map_get_list_, \
        &_##key_type##_##value_type##_map_free_, \
    }; \
    \
    _##key_type##_##value_type##_map_t* _##key_type##_##value_type##_new_map() { \
        _##key_type##_##value_type##_map_t *map = (_##key_type##_##value_type##_map_t*) malloc(sizeof(_##key_type##_##value_type##_map_t)); \
        map->buckets = vector_new(_##key_type##_##value_type##_map_match_list_t); \
        vector_push_back(map->buckets, NULL); \
        map->size = 0; \
        map->fns = &_##key_type##_##value_type##_map_v_table_; \
        map->hash = &_default_##key_type##_##value_type##_map_hash_; \
        map->key_eq = &_default_##key_type##_##value_type##_key_eq_; \
        return map; \
    }


#endif