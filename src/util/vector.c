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
        void (*pop_right)(struct _##TYPE##_vector_*);           \
        void (*push_right)(struct _##TYPE##_vector_*, TYPE);    \
        void (*resize)(struct _##TYPE##_vector_*, size_t);      \
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
    void _##TYPE##_vector_double_capacity(_##TYPE##_vector_t *vector) {  \
        vector->_array_ptr = (TYPE*) realloc(vector->_array_ptr, sizeof(TYPE) * vector->_capacity * 2);  \
        vector->_capacity *= 2;                                 \
    }                                                           \
    void _##TYPE##_vector_set(_##TYPE##_vector_t *vector, size_t index, TYPE value) {   \
        if(index + 1 < vector->_capacity) {                     \
            _##TYPE##_vector_double_capacity(vector);           \
        }                                                       \
        vector->_array_ptr[index] = value;                      \
        vector->_size += 1;                                     \
    }                                                           \
    TYPE _##TYPE##_vector_remove(_##TYPE##_vector_t *vector, size_t index) {    \
        TYPE removed_val = vector->_array_ptr[index];           \
        for(size_t i = index; i < vector->_size - 1; ++i) {     \
            vector->_array_ptr[i] = vector->_array_ptr[i + 1];  \
        }                                                       \
        return removed_val;                                     \
    }                                                           \
    void _##TYPE##_vector_pop_right(_##TYPE##_vector_t *vector) {   \
        vector->_size -= 1;                                     \
    }                                                           \
    void _##TYPE##_vector_push_right(_##TYPE##_vector_t *vector, TYPE value) {  \
        _##TYPE##_vector_set(vector, vector->_size, value);     \
    }                                                           \
    void _##TYPE##_vector_resize(_##TYPE##_vector_t *vector, size_t size) { \
        while(vector->_capacity < size) {                       \
            vector->_capacity *= 2;                             \
        }                                                       \
        vector->_array_ptr = realloc(vector->_array_ptr, vector->_capacity); \
    }                                                           \
    _##TYPE##_vector_fns_t _##TYPE##_fns = {                    \
        &_free_##TYPE##_vector, &_##TYPE##_vector_get,          \
        &_##TYPE##_vector_set, &_##TYPE##_vector_remove,        \
        &_##TYPE##_vector_pop_right, &_##TYPE##_vector_push_right, \
        &_##TYPE##_vector_resize                                \
    };                                                          \
    _##TYPE##_vector_t* _new_##TYPE##_vector() {               \
        _##TYPE##_vector_t* new_vec = (_##TYPE##_vector_t*) malloc(sizeof(_##TYPE##_vector_t)); \
        new_vec->_capacity = 1;             \
        new_vec->_size = 0;                 \
        new_vec->_array_ptr = (TYPE*) malloc(sizeof(TYPE) * new_vec->_capacity); \
        new_vec->_fns = &_##TYPE##_fns;     \
        return new_vec;                     \
    }

#ifndef UNTYPED_VECTOR_FN
#define UNTYPED_VECTOR_FN
#define Vector(TYPE)                    _##TYPE##_vector_t
#define new_vector(TYPE)                (_new_##TYPE##_vector())
#define vector_get(list, index)         ((list)->_fns->get((list), (index)))
#define vector_set(list, index, val)    ((list)->_fns->set((list), (index), (val)))
#define vector_remove(list, index)      ((list)->_fns->remove((list), (index)))
#define vector_pop_right(list)   ((list)->_fns->pop_right((list)))
#define vector_push_right(list, val)    ((list)->_fns->push_right((list), (val)))
#define vector_resize(list, size)       ((list)->_fns->resize((list), (size)))             
#endif

#endif