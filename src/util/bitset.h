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
define_set(size_t);

#define BITSET_SIZE             (4)
#define BITSET_CHUNK_SIZE       (3 + (sizeof(size_t) >> 2))
#define BITSET_INDEX(ind)       (ind >> BITSET_CHUNK_SIZE)
#define BITSET_OFFSET(ind)      (1UL << (ind & ((1 << BITSET_CHUNK_SIZE) - 1)))

struct _bitset_ {
    size_t *arr;
    size_t size;
    void *hash;             // ignore
    void *value_equals;     // ignore
};

Set(size_t)* _bitset_new();

#define GET_BS(bs, set)    _bitset_t* bs = (_bitset_t*) set->delegate

static inline void _bitset_insert_(Set(size_t)* set, size_t ind) {
    GET_BS(bs, set);
    bs->arr[BITSET_INDEX(ind)] |= BITSET_OFFSET(ind);
}

static inline size_t _bitset_erase_(Set(size_t)* set, size_t ind) {

    size_t n = bs->arr[BITSET_INDEX(ind)];
    bs->arr[BITSET_INDEX(ind)] &= ~BITSET_OFFSET(ind);
    return !!(n & BITSET_OFFSET(ind));
}

static inline size_t _bitset_count_(Set(size_t)* set, size_t ind) {
    return !!(bs->arr[BITSET_INDEX(ind)] & BITSET_OFFSET(ind));
}

static inline size_t _bitset_size_(Set(size_t)* set) {
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

static inline size_t _bitset_equals_(Set(size_t)* set1, Set(size_t)* set2) {
    if(bs1->size != bs2->size)
        return 0;
    size_t sz = bs1->size;
    for(size_t i = 0; i < sz; ++i)
        if(bs1->arr[i] != bs2->arr[i])
            return 0;
    return 1;
}

static inline List(size_t)* _bitset_get_element_list(Set(size_t)* set) {
    List(size_t) *lst = list_new(size_t);
    for(size_t i = 0; i < bs->size; ++i)
        for(size_t pow = 0; pow < (1 << BITSET_CHUNK_SIZE); ++pow)
            if(bs->arr[i] & (1 << pow)) // if pow is in
                list_push_back(lst, (i << BITSET_CHUNK_SIZE) + pow);
    return lst;
}

static inline Set(size_t)* _bitset_union_(Set(size_t)* set1, Set(size_t)* set2) {
    List(size_t) *lst = list_new(size_t);
    for(size_t i = 0; i < bs2->size; ++i)
        bs1->arr[i] |= bs2->arr[i];
    return set1;
}

static inline void _bitset_free_(Set(size_t)* set) {
    free(bs->arr);
    free(bs);
}

_size_t_set_fns_t _bitset_set_v_table_ = { 
    &_bitset_insert_, 
    &_bitset_erase_, 
    &_bitset_count_, 
    &_bitset_size_, 
    &_bitset_equals_, 
    &_bitset_get_element_list, 
    &_bitset_union_, 
    &_bitset_free_
};



Set(size_t)* _bitset_new() {
    Set(size_t) *set = set_new(size_t);
    set->delegate = malloc(sizeof(_bitset_t));
    _bitset_t *bs = (_bitset_t*) set->delegate;
    bs->arr = (size_t*) malloc(sizeof(size_t) * BITSET_SIZE);
    for(size_t i = 0; i < BITSET_SIZE; ++i) {
        bs->arr[i] = 0UL;
    }
    bs->fns = &_bitset_set_v_table_;
    bs->size = BITSET_SIZE;
    return set;
}

#endif