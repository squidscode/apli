#ifndef LIST_C
#define LIST_C

#include <stdlib.h>

/**
 * Defines a deque with the given API
 * 
 * ----- Usage -----
 * List(type) *list = list_new(type);
 *  
 */

#define List(TYPE)                  TYPE##_list_t
#define Iterator(TYPE)              TYPE##_list_node_t
#define list_get_iterator(list)          ((list)->_first)
#define iter_is_not_null(iter)      ((iter) != NULL)
#define iter_has_next(iter)         ((iter)->_next != NULL)
#define iter_next(iter)             ((iter)->_next)
#define iter_val(iter)              ((iter)->_val)
#define iter_remove(iter, list)     ((list)->_fns->_remove_node((list), (iter)))
#define list_new(TYPE)              (_new_##TYPE##_list())
#define list_size(list)             ((list)->_fns->_get_size(list))
#define list_get_front(list)        ((list)->_fns->_get_first((list)))
#define list_get_first(list)        ((list)->_fns->_get_first((list)))
#define list_get_back(list)         ((list)->_fns->_get_last((list)))
#define list_get_last(list)         ((list)->_fns->_get_last((list)))
#define list_push_front(list, val)  ((list)->_fns->_push_front((list), (val)))
#define list_push_back(list, val)   ((list)->_fns->_push_back((list), (val)))
#define list_pop_front(list)        ((list)->_fns->_pop_front((list)))
#define list_pop_back(list)         ((list)->_fns->_pop_back((list)))
#define list_free(list)             ((list)->_fns->_free((list)))

#define define_list(TYPE)                       \
    struct _##TYPE##_list_;                     \
    typedef struct _##TYPE##_list_node_ {       \
        TYPE _val;                              \
        struct _##TYPE##_list_node_ *_next;     \
        struct _##TYPE##_list_node_ *_prev;     \
    } TYPE##_list_node_t;                       \
    typedef struct _##TYPE##_list_fns_ {        \
        TYPE (*_get_first)(struct _##TYPE##_list_*); \
        TYPE (*_get_last)(struct _##TYPE##_list_*);  \
        void (*_push_front)(struct _##TYPE##_list_*, TYPE); \
        void (*_push_back)(struct _##TYPE##_list_*, TYPE);  \
        void (*_pop_front)(struct _##TYPE##_list_*); \
        void (*_pop_back)(struct _##TYPE##_list_*);  \
        void (*_free)(struct _##TYPE##_list_*);      \
        size_t (*_get_size)(struct _##TYPE##_list_*);     \
        void (*_remove_node)(struct _##TYPE##_list_*, TYPE##_list_node_t*); \
    } TYPE##_list_fns_t;                          \
    typedef struct _##TYPE##_list_ {            \
        size_t _size;                           \
        TYPE##_list_node_t *_first;             \
        TYPE##_list_node_t *_last;              \
        TYPE##_list_fns_t *_fns;                \
    } TYPE##_list_t;                                 \
    void _##TYPE##_list_node_remove(TYPE##_list_t *list, TYPE##_list_node_t *node) { \
        if(node == list->_first) { \
            list->_fns->_pop_front(list); \
        } else if(node == list->_last) { \
            list->_fns->_pop_back(list); \
        } else { \
            node->_prev->_next = node->_next; \
            node->_next->_prev = node->_prev; \
            list->_size -= 1; \
            free(node); \
        } \
    } \
    static inline TYPE _##TYPE##_list_get_first(TYPE##_list_t *list) {    \
        return list->_first->_val;                          \
    }                                                       \
    static inline TYPE _##TYPE##_list_get_last(TYPE##_list_t *list) {     \
        return list->_last->_val;                           \
    }                                                       \
    static inline void _##TYPE##_list_push_front(TYPE##_list_t *list,     \
        TYPE val) {                                         \
        TYPE##_list_node_t *new_node = (TYPE##_list_node_t*)  \
            malloc(sizeof(TYPE##_list_node_t));               \
        new_node->_val  = val;                              \
        new_node->_next = list->_first;                     \
        /* Non-branching instructions: */ \
        list->_last = (TYPE##_list_node_t*) \
                     ((size_t)(list->_size == 0) * (size_t)new_node     \
                    + (size_t)(list->_size != 0) * (size_t)list->_last); \
        TYPE##_list_node_t* proxy = (TYPE##_list_node_t*) \
                         ((size_t)(list->_size == 0) * (size_t)new_node \
                        + (size_t)(list->_size != 0) * (size_t)list->_first); \
        proxy->_prev = (TYPE##_list_node_t*) \
                             ((size_t)(list->_size != 0) * (size_t)new_node \
                            + (size_t)(list->_size == 0) * (size_t)proxy->_prev); \
        new_node->_prev = NULL;                             \
        list->_first = new_node; \
        ++list->_size;                                   \
    }                                                       \
    static inline void _##TYPE##_list_push_back(TYPE##_list_t *list,      \
        TYPE val) {                                         \
        TYPE##_list_node_t *new_node = (TYPE##_list_node_t*)  \
            malloc(sizeof(TYPE##_list_node_t));               \
        new_node->_val     = val;                           \
        new_node->_prev    = list->_last;                   \
        list->_first = (TYPE##_list_node_t*) \
                     ((size_t)(list->_size == 0) * (size_t)new_node     \
                    + (size_t)(list->_size != 0) * (size_t)list->_first); \
        TYPE##_list_node_t* proxy = (TYPE##_list_node_t*) \
                                 ((size_t)(list->_size == 0) * (size_t)new_node \
                                + (size_t)(list->_size != 0) * (size_t)list->_last); \
        proxy->_next = (TYPE##_list_node_t*) \
                             ((size_t)(list->_size != 0) * (size_t)new_node \
                            + (size_t)(list->_size == 0) * (size_t)proxy->_next); \
        new_node->_next = NULL; \
        list->_last = new_node; \
        ++list->_size;\
    }                                                       \
    void _##TYPE##_list_pop_front(TYPE##_list_t *list) {    \
        --list->_size; \
        TYPE##_list_node_t *node_after = (TYPE##_list_node_t*) \
             ((size_t)(list->_size == 0) * (size_t)list->_first     \
            + (size_t)(list->_size != 0) * (size_t)list->_first->_next); \
        list->_last = (TYPE##_list_node_t*) \
                     ((size_t)(list->_size == 0) * (size_t)NULL     \
                    + (size_t)(list->_size != 0) * (size_t)list->_last); \
        node_after->_prev = NULL; \
        free(list->_first); \
        list->_first = (TYPE##_list_node_t*) \
                     ((size_t)(list->_size == 0) * (size_t)NULL     \
                    + (size_t)(list->_size != 0) * (size_t)node_after); \
    }                                                       \
    void _##TYPE##_list_pop_back(TYPE##_list_t *list) {     \
        --list->_size; \
        TYPE##_list_node_t *node_before = (TYPE##_list_node_t*) \
             ((size_t)(list->_size == 0) * (size_t)list->_last     \
            + (size_t)(list->_size != 0) * (size_t)list->_last->_prev); \
        list->_first = (TYPE##_list_node_t*) \
                     ((size_t)(list->_size == 0) * (size_t)NULL     \
                    + (size_t)(list->_size != 0) * (size_t)list->_first); \
        node_before->_next = NULL; \
        free(list->_last); \
        list->_last = (TYPE##_list_node_t*) \
                     ((size_t)(list->_size == 0) * (size_t)NULL     \
                    + (size_t)(list->_size != 0) * (size_t)node_before); \
    }                                                       \
    void _free_##TYPE##_list(TYPE##_list_t *list) {         \
        TYPE##_list_node_t *ptr = list->_first; \
        TYPE##_list_node_t *tmp;                \
        while(list->_size) {                            \
            tmp = ptr->_next; \
            free(ptr); \
            ptr = tmp; \
            --list->_size; \
        }                                                   \
        free(list);                                         \
    }                                                       \
    static inline size_t _##TYPE##_list_get_size(TYPE##_list_t *list) {   \
        return list->_size; \
    } \
    TYPE##_list_fns_t TYPE##_list_fns = {                     \
        &_##TYPE##_list_get_first, &_##TYPE##_list_get_last,    \
        &_##TYPE##_list_push_front, &_##TYPE##_list_push_back,  \
        &_##TYPE##_list_pop_front, &_##TYPE##_list_pop_back,    \
        &_free_##TYPE##_list, &_##TYPE##_list_get_size,         \
        &_##TYPE##_list_node_remove                             \
    };                                                          \
    TYPE##_list_t* _new_##TYPE##_list() {                   \
        TYPE##_list_t *new_list = (TYPE##_list_t*) malloc(sizeof(TYPE##_list_t));  \
        new_list->_size = 0;                                \
        new_list->_first = NULL;                            \
        new_list->_last = NULL;                             \
        new_list->_fns = &TYPE##_list_fns;                   \
        return new_list;                                    \
    }                                                       \



#endif