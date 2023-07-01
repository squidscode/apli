#ifndef SET_H
#define SET_H

#include <stdlib.h>
#include <assert.h>
#include "vector.h"
#include "list.h"

// Macros
#define RESIZE_RATIO    0.80

/**
 *
 * Defines a set with the given type. 
 * 
 * ----- Usage -----
 * Set(type) *set = set_new(type);
 *   set_set_hash(set, fn)             -> void
 *   set_set_value_equals(set, fn)     -> void
 *   set_insert(set, value)            -> void
 *   set_erase(set, value)             -> size_t
 *   set_count(set, value)             -> size_t
 *   set_size(set)                     -> size_t
 *   set_equals(set1, set2)            -> size_t
 *   set_free(set)                     -> void
 *
 * SetIterator(type) *iter = set_iterator(set);
 *   set_iterator_is_null(set_iterator) -> size_t
 *   set_iterator_next(set_iterator)    -> SetIterator(type)
 */

#ifndef UNTYPED_SET_FN
#define UNTYPED_SET_FN
#define Set(type)                                    _##type##_set_t
#define set_new(type)                               (_##type##_new_set())
#define set_set_value_equals(set, fn_ref)           ((set)->value_equals = (fn_ref))
#define set_set_hash(set, fn_ref)                   ((set)->hash = (fn_ref))
#define set_insert(set, value)                      ((set)->fns->insert((set), (value)))
#define set_erase(set, value)                       ((set)->fns->erase((set), (value)))
#define set_count(set, value)                       ((set)->fns->count((set), (value)))
#define set_size(set)                               ((set)->fns->size((set)))
#define set_equals(set1, set2)                      ((set1)->fns->equals((set1), (set2)))
#define set_free(set)                               ((set)->fns->free((set)))
#define SetIterator(type)
#define set_iterator(set)                           ((set)->fns->iterator((set)))
#define set_iterator_is_null(iterator)
#define set_iterator_next(iterator)
#endif

#define define_set_hash(type) \
    size_t _##type##_set_collection_hash(_##type##_set_t *set) { \
        size_t hash_total = 0; \
        for(size_t ind = 0; ind < buckets_size; ++ind) { \
            _##type##_set_match_list_t bucket = vector_get(set2->buckets, ind); \
            if(bucket == NULL || list_size(bucket) == 0) continue; \
            Iterator(_##type##_set_match_t) *iter = get_iterator(bucket); \
            while(iter != NULL) { \
                hash_total ^= _default_##type##_set_hash_(iter_val(iter)); \
                iter = iter_next(iter); \
            } \
        } \
        return hash_total; \
    }

#define define_set(type) \
    struct _##type##_set_; \
    /* Virtual function table for dynamic dispatch (polymorphism) */ \
    typedef struct _##type##_set_fns_ { \
        void (*insert)(struct _##type##_set_*, type); \
        size_t (*erase)(struct _##type##_set_*, type); \
        size_t (*count)(struct _##type##_set_*, type); \
        size_t (*size)(struct _##type##_set_*); \
        size_t (*equals)(struct _##type##_set_*, struct _##type##_set_*); \
        void (*free)(struct _##type##_set_*); \
    } _##type##_set_fns_t; \
    \
    /* A `set_match` holds information about a single value. */ \
    typedef struct _##type##_set_match_ { \
        size_t hash; \
        type value; \
    } _##type##_set_match_t; \
    /* List+Vector definitions for buckets of `set_match`s. */ \
    define_list(_##type##_set_match_t); \
    typedef List(_##type##_set_match_t)* _##type##_set_match_list_t; \
    define_vector(_##type##_set_match_list_t); \
    \
    /* A `set` contains a hash function, buckets of `set_match`, the size of the set, */ \
    typedef struct _##type##_set_ { \
        size_t (*hash)(type); \
        size_t (*value_equals)(type, type); \
        Vector(_##type##_set_match_list_t) *buckets; \
        size_t size; \
        _##type##_set_fns_t *fns; \
    } _##type##_set_t; \
    \
    /* The default hash function for the set. */ \
    size_t _default_##type##_set_hash_(type key) { \
        size_t size = (sizeof(type) / sizeof(char)); \
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
    /* The default value_equals function. */ \
    size_t _default_##type##_value_equals_(type key1, type key2) { \
        size_t size = (sizeof(type) / sizeof(char)); \
        char *ptr1 = (char*) ((void*) &key1); \
        char *ptr2 = (char*) ((void*) &key2); \
        for(size_t i = 0; i < size; ++i) { \
            if(ptr1[i] != ptr2[i]) return 0; \
        } \
        return 1; \
    } \
    \
    /* Allocates the (hash, key, value) into the appropriate bucket in the set. */ \
    void _allocate_into_bucket_##type##_set_(_##type##_set_t *set, size_t hash, type val) { \
        size_t mask = (vector_size(set->buckets) - 1); \
        _##type##_set_match_list_t bucket = vector_get(set->buckets, (hash & mask)); \
        if(bucket == NULL) { \
            bucket = list_new(_##type##_set_match_t); \
        } else { \
            set->fns->erase(set, val); \
        } \
        _##type##_set_match_t match = {hash, val}; \
        list_push_back(bucket, match); \
        vector_set(set->buckets, (hash & mask), bucket); \
    } \
    \
    /* Check AND resize the set if the size of the set exceeds RESIZE_RATIO. */ \
    void _check_resize_##type##_set_(_##type##_set_t *set) { \
        size_t buckets_size = vector_size(set->buckets); \
        if (set->size < RESIZE_RATIO * buckets_size) return; \
        size_t new_cap = (buckets_size << 1); \
        vector_resize_val(set->buckets, new_cap, NULL); \
        for(size_t i = 0; i < buckets_size; ++i) { \
            size_t mask = (new_cap - 1); \
            _##type##_set_match_list_t bucket = vector_get(set->buckets, i); \
            if(bucket == NULL) \
                continue; \
            size_t list_size = list_size(bucket); \
            for(size_t ind = 0; ind < list_size; ++ind) { \
                _##type##_set_match_t match = list_get_first(bucket); \
                list_pop_front(bucket); \
                _allocate_into_bucket_##type##_set_(set, match.hash, match.value); \
            } \
        } \
    } \
    \
    void _##type##_set_insert_(_##type##_set_t *set, type val) { \
        _allocate_into_bucket_##type##_set_( \
            set, \
            set->hash(val), \
            val);\
        set->size += 1; \
        _check_resize_##type##_set_(set); \
    } \
    \
    size_t _##type##_set_erase_(_##type##_set_t *set, type val) { \
        size_t mask = (vector_size(set->buckets) - 1); \
        size_t val_hash = set->hash(val); \
        _##type##_set_match_list_t bucket = vector_get(set->buckets, (val_hash & mask)); \
        if(bucket == NULL || list_size(bucket) == 0) return 0; \
        Iterator(_##type##_set_match_t) *iter = get_iterator(bucket); \
        while(iter != NULL) { \
            _##type##_set_match_t match = iter_val(iter); \
            if(match.hash == val_hash && set->value_equals(match.value, val)) { \
                iter_remove(iter, bucket); \
                set->size -= 1; \
                return 1; \
            }; \
            iter = iter_next(iter); \
        } \
        return 0; \
    } \
    \
    size_t _##type##_set_count_(_##type##_set_t *set, type val) { \
        size_t mask = (vector_size(set->buckets) - 1); \
        size_t val_hash = set->hash(val); \
        _##type##_set_match_list_t bucket = vector_get(set->buckets, (val_hash & mask)); \
        if(bucket == NULL || list_size(bucket) == 0) return 0; \
        Iterator(_##type##_set_match_t) *iter = get_iterator(bucket); \
        while(iter != NULL) { \
            _##type##_set_match_t match = iter_val(iter); \
            if(match.hash == val_hash && set->value_equals(match.value, val)) \
                return 1; \
            iter = iter_next(iter); \
        } \
        return 0; \
    } \
    \
    size_t _##type##_set_size_(_##type##_set_t *set) { \
        return set->size; \
    } \
    \
    size_t _##type##_set_contains_all_(_##type##_set_t *set1, _##type##_set_t *set2) { \
        size_t buckets_size = vector_size(set2->buckets); \
        for(size_t ind = 0; ind < buckets_size; ++ind) { \
            _##type##_set_match_list_t bucket = vector_get(set2->buckets, ind); \
            if(bucket == NULL || list_size(bucket) == 0) continue; \
            Iterator(_##type##_set_match_t) *iter = get_iterator(bucket); \
            while(iter != NULL) { \
                _##type##_set_match_t match = iter_val(iter); \
                if(0 == set_count(set1, match.value)) \
                    return 0; \
                iter = iter_next(iter); \
            } \
        } \
        return 1; \
    } \
    \
    size_t _##type##_set_equals_(_##type##_set_t *set1, _##type##_set_t *set2) { \
        return _##type##_set_contains_all_(set1, set2) \
            && _##type##_set_contains_all_(set2, set1); \
    } \
    \
    void _##type##_set_free_(_##type##_set_t *set) { \
        size_t buckets_size = vector_size(set->buckets); \
        for(size_t ind = 0; ind < buckets_size; ++ind) { \
            _##type##_set_match_list_t bucket = vector_get(set->buckets, ind); \
            if(bucket != NULL) list_free(bucket); \
        } \
        vector_free(set->buckets); \
        free(set); \
    } \
    \
    _##type##_set_fns_t _##type##_set_v_table_ = { \
        &_##type##_set_insert_, \
        &_##type##_set_erase_, \
        &_##type##_set_count_, \
        &_##type##_set_size_, \
        &_##type##_set_equals_, \
        &_##type##_set_free_, \
    }; \
    \
    _##type##_set_t* _##type##_new_set() { \
        _##type##_set_t *set = (_##type##_set_t*) malloc(sizeof(_##type##_set_t)); \
        set->buckets = vector_new(_##type##_set_match_list_t); \
        vector_push_back(set->buckets, NULL); \
        set->size = 0; \
        set->fns = &_##type##_set_v_table_; \
        set->hash = &_default_##type##_set_hash_; \
        set->value_equals = &_default_##type##_value_equals_; \
        return set; \
    }

#endif