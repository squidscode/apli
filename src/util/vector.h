#ifndef VECTOR_C
#define VECTOR_C

#include <stdlib.h>

#define define_vector(TYPE)                 \
    struct _##TYPE##_vector_;               \
    typedef struct _##TYPE##_vector_fns_ {  \
        void (*free)(struct _##TYPE##_vector_*);                \
        TYPE (*get)(struct _##TYPE##_vector_*, size_t);         \
        void (*set)(struct _##TYPE##_vector_*, size_t, TYPE);   \
        TYPE (*remove)(struct _##TYPE##_vector_*, size_t);      \
        void (*pop_back)(struct _##TYPE##_vector_*);           \
        void (*push_back)(struct _##TYPE##_vector_*, TYPE);    \
        void (*resize)(struct _##TYPE##_vector_*, size_t, TYPE);      \
        size_t (*size)(struct _##TYPE##_vector_*);              \
    } _##TYPE##_vector_fns_t;               \
    typedef struct _##TYPE##_vector_ {      \
        unsigned int _capacity;             \
        unsigned int _size;                 \
        TYPE *_array_ptr;                   \
        _##TYPE##_vector_fns_t* _fns;       \
    } _##TYPE##_vector_t;                   \
    void _free_##TYPE##_vector(_##TYPE##_vector_t *vector) {    \
        free(vector->_array_ptr);                               \
        free(vector);                                           \
    }                                                           \
    TYPE _##TYPE##_vector_get(_##TYPE##_vector_t *vector, size_t index) {    \
        return vector->_array_ptr[index];                       \
    }                                                           \
    void _##TYPE##_vector_double_capacity(_##TYPE##_vector_t *vector) { \
        TYPE *new_ptr = (TYPE*) malloc((sizeof(TYPE) * vector->_capacity) << 1); \
        for(size_t i = 0; i < vector->_size; ++i) { \
            new_ptr[i] = vector->_array_ptr[i]; \
        } \
        free(vector->_array_ptr); \
        vector->_array_ptr = new_ptr; \
        vector->_capacity = vector->_capacity << 1;                                 \
    }                                                           \
    void _##TYPE##_vector_set(_##TYPE##_vector_t *vector, size_t index, TYPE value) {   \
        vector->_array_ptr[index] = value;                      \
    }                                                           \
    TYPE _##TYPE##_vector_remove(_##TYPE##_vector_t *vector, size_t index) {    \
        TYPE removed_val = vector->_array_ptr[index];           \
        for(size_t i = index; i < vector->_size - 1; ++i) {     \
            vector->_array_ptr[i] = vector->_array_ptr[i + 1];  \
        }                                                       \
        vector->_size -= 1;                                     \
        return removed_val;                                     \
    }                                                           \
    void _##TYPE##_vector_pop_back(_##TYPE##_vector_t *vector) {   \
        vector->_size -= 1;                                     \
    }                                                           \
    void _##TYPE##_vector_push_back(_##TYPE##_vector_t *vector, TYPE value) {  \
        if(vector->_capacity < vector->_size + 1) _##TYPE##_vector_double_capacity(vector); \
        vector->_array_ptr[vector->_size] = value; \
        vector->_size += 1; \
    }                                                           \
    void _##TYPE##_vector_resize(_##TYPE##_vector_t *vector, size_t size, TYPE fill) { \
        TYPE *new_ptr = (TYPE*) malloc(sizeof(TYPE) * (size <= 0 ? 1 : size)); \
        for(size_t i = 0; i < (vector->_size < size ? vector->_size : size); ++i) { \
            new_ptr[i] = vector->_array_ptr[i]; \
        } \
        free(vector->_array_ptr); \
        if(vector->_size < size) for(size_t i = vector->_size; i < size; ++i) { \
            new_ptr[i] = fill; \
        } \
        vector->_array_ptr = new_ptr; \
        vector->_capacity = (size <= 0 ? 1 : size); \
        vector->_size = size; \
    }                                                           \
    size_t _##TYPE##_get_size(_##TYPE##_vector_t *vector) {     \
        return vector->_size;                                   \
    }                                                           \
    _##TYPE##_vector_fns_t _##TYPE##_fns = {                    \
        &_free_##TYPE##_vector, &_##TYPE##_vector_get,          \
        &_##TYPE##_vector_set, &_##TYPE##_vector_remove,        \
        &_##TYPE##_vector_pop_back, &_##TYPE##_vector_push_back, \
        &_##TYPE##_vector_resize, &_##TYPE##_get_size           \
    };                                                          \
    _##TYPE##_vector_t* _new_##TYPE##_vector() {               \
        _##TYPE##_vector_t* new_vec = (_##TYPE##_vector_t*) malloc(sizeof(_##TYPE##_vector_t)); \
        new_vec->_capacity = 1;             \
        new_vec->_size = 0;                 \
        new_vec->_array_ptr = (TYPE*) malloc(sizeof(TYPE)); \
        new_vec->_fns = &_##TYPE##_fns;     \
        return new_vec;                     \
    }

#ifndef UNTYPED_VECTOR_FN
#define UNTYPED_VECTOR_FN
#define Vector(TYPE)                        _##TYPE##_vector_t
#define new_vector(TYPE)                    (_new_##TYPE##_vector())
#define vector_get(vec, index)              ((vec)->_fns->get((vec), (index)))
#define vector_set(vec, index, val)         ((vec)->_fns->set((vec), (index), (val)))
#define vector_remove(vec, index)           ((vec)->_fns->remove((vec), (index)))
#define vector_pop_back(vec)                ((vec)->_fns->pop_back((vec)))
#define vector_push_back(vec, val)          ((vec)->_fns->push_back((vec), (val)))
#define vector_resize(vec, size)            ((vec)->_fns->resize((vec), (size), 0))
#define vector_resize_val(vec, size, val)   ((vec)->_fns->resize((vec), (size), (val)))             
#define vector_size(vec)                    ((vec)->_fns->size((vec)))
#define vector_free(vec)                    ((vec)->_fns->free((vec)))
#endif

#endif