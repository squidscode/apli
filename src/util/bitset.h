#ifndef BITSET_H
#define BITSET_H

#include <stdlib.h>
#include <assert.h>
#include "set.h"
#include "vector.h"

// #define Set(type)                                    _##type##_set_t
// #define set_new(type)                               (_##type##_new_set())
// #define set_set_value_equals(set, fn_ref)           ((set)->value_equals = (fn_ref))
// #define set_set_hash(set, fn_ref)                   ((set)->hash = (fn_ref))
// #define set_insert(set, value)                      ((set)->fns->insert((set), (value)))
// #define set_erase(set, value)                       ((set)->fns->erase((set), (value)))
// #define set_count(set, value)                       ((set)->fns->count((set), (value)))
// #define set_size(set)                               ((set)->fns->size((set)))
// #define set_equals(set1, set2)                      ((set1)->fns->equals((set1), (set2)))
// #define set_union(set1, set2)                       ((set1)->fns->_union((set1), (set2)))
// #define set_free(set)                               ((set)->fns->destroy((set)))
// #define set_get_list(set)                           ((set)->fns->get_element_list((set)))

#define bitset_new()                                    (_bitset_new())

typedef struct _bitset_ _bitset_t;

define_list(size_t);

#define BITSET_SIZE             (4)
#define BITSET_CHUNK_SIZE       (3 + (sizeof(size_t) >> 2))
#define BITSET_INDEX(ind)       (ind >> BITSET_CHUNK_SIZE)
#define BITSET_OFFSET(ind)      (1UL << (ind & ((1 << BITSET_CHUNK_SIZE) - 1)))

typedef struct _bitset_fns_ _bitset_fns_t;
struct _bitset_ {
    size_t *arr;
    size_t size;
    _bitset_fns_t *fns;
    void *hash;             // ignore
    void *value_equals;     // ignore
};
typedef struct _bitset_ _bitset_t;

typedef struct _bitset_fns_ {
    void (*insert)(_bitset_t*, size_t);
    size_t (*erase)(_bitset_t*, size_t);
    size_t (*count)(_bitset_t*, size_t);
    size_t (*size)(_bitset_t*);
    size_t (*equals)(_bitset_t*, _bitset_t*);
    List(size_t)* (*get_element_list)(_bitset_t*);
    _bitset_t* (*_union)(_bitset_t*, _bitset_t*);
    void (*destroy)(_bitset_t*);
} _bitset_fns_t;

_bitset_t* _bitset_new();

static inline size_t _bitset_collection_hash(_bitset_t *bs) {
    size_t hash_total = 0;
    for(size_t i = 0; i < bs->size; ++i)
        hash_total ^= bs->arr[i];
    return hash_total;
}

static inline void _bitset_insert_(_bitset_t* bs, size_t ind) {
#ifndef NO_ASSERT
    assert(BITSET_INDEX(ind) < BITSET_SIZE); // Not a significant slowdown
#endif
    bs->arr[BITSET_INDEX(ind)] |= BITSET_OFFSET(ind);
}

static inline size_t _bitset_erase_(_bitset_t* bs, size_t ind) {
    size_t n = bs->arr[BITSET_INDEX(ind)];
    bs->arr[BITSET_INDEX(ind)] &= ~BITSET_OFFSET(ind);
    return !!(n & BITSET_OFFSET(ind));
}

static inline size_t _bitset_count_(_bitset_t* bs, size_t ind) {
    return !!(bs->arr[BITSET_INDEX(ind)] & BITSET_OFFSET(ind));
}

static inline size_t _bitset_size_(_bitset_t* bs) {
    size_t total = 0;
    for(size_t i = 0; i < bs->size; ++i) {
        size_t n = bs->arr[i];
        while(n) {
            n &= n - 1;
            ++total;
        }
    }
    return total;
}

static inline size_t _bitset_equals_(_bitset_t* bs1, _bitset_t* bs2) {
    if(bs1->size != bs2->size)
        return 0;
    size_t sz = bs1->size;
    for(size_t i = 0; i < sz; ++i)
        if(bs1->arr[i] != bs2->arr[i])
            return 0;
    return 1;
}

static inline List(size_t)* _bitset_get_element_list(_bitset_t* bs) {
    List(size_t) *lst = list_new(size_t);
    for(size_t i = 0; i < bs->size; ++i)
        for(size_t pow = 0; pow < (1 << BITSET_CHUNK_SIZE); ++pow)
            if(bs->arr[i] & (1 << pow)) // if pow is in
                list_push_back(lst, (i << BITSET_CHUNK_SIZE) + pow);
    return lst;
}

static inline _bitset_t* _bitset_union_(_bitset_t* bs1, _bitset_t* bs2) {
    for(size_t i = 0; i < bs2->size; ++i)
        bs1->arr[i] |= bs2->arr[i];
    return bs1;
}

static inline void _bitset_free_(_bitset_t* bs) {
    free(bs->arr);
    free(bs);
}

_bitset_fns_t _bitset_set_v_table_ = { 
    &_bitset_insert_, 
    &_bitset_erase_, 
    &_bitset_count_, 
    &_bitset_size_, 
    &_bitset_equals_, 
    &_bitset_get_element_list, 
    &_bitset_union_, 
    &_bitset_free_
};



_bitset_t* _bitset_new() {
    _bitset_t *bs = (_bitset_t*) malloc(sizeof(_bitset_t));
    bs->arr = (size_t*) malloc(sizeof(size_t) * BITSET_SIZE);
    for(size_t i = 0; i < BITSET_SIZE; ++i) {
        bs->arr[i] = 0UL;
    }
    bs->fns = &_bitset_set_v_table_;
    bs->size = BITSET_SIZE;
    return bs;
}

#endif