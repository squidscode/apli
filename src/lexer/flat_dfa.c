#include <assert.h>
#include <stdlib.h>
#include "dfa.h"
#include "../util/list.h"

#ifndef SIZE_T_LIST
    #define SIZE_T_LIST
    define_list(char);
#endif

define_list(size_t);
define_set(size_t);
init_dfa_types(size_t, char);
define_dfa(size_t, char);

#define _flat_dfa_offset_constant                               (7UL)
#define _flat_dfa_begin_state                                   (0)
#define _flat_dfa_state_exists(state)                           ((state) + 1)
#define _flat_dfa_state_is_accept(state)                        ((state) & 1)
#define _flat_dfa_offset_into_transition(state, transition)     (((size_t) (state) << (_flat_dfa_offset_constant)) + (255UL & transition))

#define flat_dfa_new(num_states)                                (_flat_dfa_new(num_states))
#define flat_dfa_serialize(dfa)                                 (_flat_dfa_serialize(dfa))
#define flat_dfa_deserialize(ptr)                               (_flat_dfa_deserialize(ptr))
#define flat_dfa_from_compressed_dfa(dfa)                       (_flat_dfa_from_compressed_dfa(dfa))
struct _flat_dfa_ ;

/* Virtual table for mutable DFA functions */
struct _flat_dfa_fns_ {
    size_t (*run)(struct _flat_dfa_*, Iterator(char)*);
    size_t (*run_greedy)(struct _flat_dfa_*, const char*, size_t);
    size_t (*run_greedy_iter)(struct _flat_dfa_*, Iterator(char)*);
    void (*add_transition)(struct _flat_dfa_*, size_t, char, size_t);
    size_t (*remove_transition)(struct _flat_dfa_*, size_t, char);
    void (*add_accept_state)(struct _flat_dfa_*, size_t);
    size_t (*remove_accept_state)(struct _flat_dfa_*, size_t);
    void (*destroy)(struct _flat_dfa_*);
};
typedef struct _flat_dfa_fns_ _flat_dfa_fns_t;

/* A struct that represents a dfa. */
struct _flat_dfa_ {
    char *transition;
    size_t state_size;
    _flat_dfa_fns_t *fns;
};
typedef struct _flat_dfa_ _flat_dfa_t;
_flat_dfa_t* _flat_dfa_new(size_t state_size);

/* Throws an assert error if the dfa is locked. */
inline void _flat_assert_is_not_locked(_flat_dfa_t *dfa) { }

/* Runs the dfa with the given transition iterator. */
size_t _flat_dfa_run_greedy(_flat_dfa_t *dfa, const char *ptr, size_t ptr_sz) {
    size_t current_state = _flat_dfa_begin_state;
    size_t offset = 0UL;
    size_t max_right_bound = ~0UL;
    size_t state = 0;
    while(offset <= ptr_sz) {
        size_t real_state = current_state >> 1;        
        if (_flat_dfa_state_exists(
            dfa->transition[_flat_dfa_offset_into_transition(real_state, ptr[offset])]
        )) {
            current_state = dfa->transition[_flat_dfa_offset_into_transition(real_state, ptr[offset])];
        } else {
            current_state = _flat_dfa_begin_state;
        }
        ++offset;

        /* printf("OFFSET: %zu, STATE: %zu\n", offset, current_state); */
        if(real_state == _flat_dfa_begin_state && max_right_bound != ~0UL) {
            /* printf("RETURNING\n"); */
            return max_right_bound;\
        } else if(_flat_dfa_state_is_accept(current_state)) {
            /* printf("State is accepting!\n"); */
            max_right_bound = offset;
        }
    }
    return max_right_bound;
}

/* Runs the dfa with the given transition iterator. */
size_t _flat_dfa_run_greedy_iterator(_flat_dfa_t *dfa, Iterator(char) *transition_iter) {
    size_t current_state = _flat_dfa_begin_state;
    Iterator(char) *iter_ptr = transition_iter;
    size_t offset = 0UL;
    size_t max_right_bound = ~0UL;
    size_t state = 0;
    while(iter_ptr != NULL) {
        size_t real_state = current_state >> 1;
        if (_flat_dfa_state_exists(
            dfa->transition[_flat_dfa_offset_into_transition(real_state, iter_val(iter_ptr))]
        )) {
            current_state = dfa->transition[_flat_dfa_offset_into_transition(real_state, iter_val(iter_ptr))];
        } else {
            current_state = _flat_dfa_begin_state;
        }
        iter_ptr = iter_next(iter_ptr);
        ++offset;
        
        /* printf("OFFSET: %zu, STATE: %zu\n", offset, current_state); */
        if(real_state == _flat_dfa_begin_state && max_right_bound != ~0UL) {
            /* printf("RETURNING\n"); */
            return max_right_bound;\
        } else if(_flat_dfa_state_is_accept(current_state)) {
            /* printf("State is accepting!\n"); */
            max_right_bound = offset;
        }
    }
    return max_right_bound;
}

/* Runs the dfa with the given transition iterator. */
size_t _flat_dfa_run(_flat_dfa_t *dfa, Iterator(char) *transition_iter) {
    size_t current_state = _flat_dfa_begin_state;
    while(transition_iter != NULL) {
        size_t real_state = current_state >> 1;
        // printf("Transition (%zu, %c) ", real_state, iter_val(transition_iter));
        // printf("offset: %zu ", _flat_dfa_offset_into_transition(real_state, iter_val(transition_iter)));
        // printf("transition[offset]: %i\n", dfa->transition[_flat_dfa_offset_into_transition(real_state, iter_val(transition_iter))]);
        if (_flat_dfa_state_exists(
            dfa->transition[_flat_dfa_offset_into_transition(real_state, iter_val(transition_iter))]
        ))
            current_state = dfa->transition[_flat_dfa_offset_into_transition(real_state, iter_val(transition_iter))];
        else
            return 0;
        transition_iter = iter_next(transition_iter);
    }
    return _flat_dfa_state_is_accept(current_state);
}

void _flat_dfa_add_transition(_flat_dfa_t *dfa, size_t from, char transition, size_t to) {
    /* printf("Transition (%i, %c) hash: %llu\n", dfa->tmp_trans.state, dfa->tmp_trans.transition, dfa->transition_map->hash(dfa->tmp_trans)); */
    // printf("Adding transition (%zu, %i)! \n", from, transition);
    // printf("offset: %zu ", _flat_dfa_offset_into_transition(from, transition));
    // printf("transition[offset]: %i\n", dfa->transition[_flat_dfa_offset_into_transition(from, transition)]);
    dfa->transition[_flat_dfa_offset_into_transition(from, transition)] = (to << 1);
}

size_t _flat_dfa_remove_transition(_flat_dfa_t *dfa, size_t state, char transition) {
    size_t tmp = dfa->transition[_flat_dfa_offset_into_transition(state, transition)];
    dfa->transition[_flat_dfa_offset_into_transition(state, transition)] = ~0;
    return _flat_dfa_state_exists(tmp);
}

void _flat_dfa_add_accepting_state(_flat_dfa_t *dfa, size_t state) {
    for(size_t i = 0; i < dfa->state_size << _flat_dfa_offset_constant; ++i) {
        if(_flat_dfa_state_exists(dfa->transition[i]) && (dfa->transition[i] >> 1) == state) {
            dfa->transition[i] |= 1;
        }
    }
}

size_t _flat_dfa_remove_accepting_state(_flat_dfa_t *dfa, size_t state) {
    for(size_t i = 0; i < dfa->state_size << _flat_dfa_offset_constant; ++i) {
        if(_flat_dfa_state_exists(dfa->transition[i]) && (dfa->transition[i] >> 1) == state) {
            dfa->transition[i] &= ~1;
        }
    }
    return 1UL;
}

void _flat_dfa_free(_flat_dfa_t *dfa) {
    free(dfa->transition);
    free(dfa);
}

const char* _flat_dfa_serialize(_flat_dfa_t* dfa) {
    void *ptr = (char*) malloc((dfa->state_size << _flat_dfa_offset_constant) + (sizeof(size_t) << 1));
    ((size_t*) ptr)[0] = _flat_dfa_offset_constant;
    ((size_t*) ptr)[1] = dfa->state_size;
    size_t offset = (sizeof(size_t) << 1);
    for(size_t i = 0; i < dfa->state_size << _flat_dfa_offset_constant; ++i) {
        ((char*) ptr)[offset + i] = dfa->transition[i];
    }
    return (const char*) ptr;
}

_flat_dfa_t* _flat_dfa_from_compressed_dfa(Dfa(size_t, char) *dfa) {
    assert(dfa->begin_state == 0); // Only supports begin state == 0. 
    size_t max_state = 0UL;
    __size_t_char_transition_t_size_t_map_match_t_list_t *matches = map_get_list(dfa->transition_map);
    while(list_size(matches)) {
        __size_t_char_transition_t_size_t_map_match_t match = list_get_front(matches);
        if(max_state < match.key.state)
            max_state = match.key.state;
        if(max_state < match.value)
            max_state = match.value;
        list_pop_front(matches);
    }
    list_free(matches);

    _flat_dfa_t *flat_dfa = flat_dfa_new(max_state + 1); 
    matches = map_get_list(dfa->transition_map);
    while(list_size(matches)) {
        __size_t_char_transition_t_size_t_map_match_t match = list_get_front(matches);
        dfa_add_transition(flat_dfa, match.key.state, match.key.transition, match.value);
        list_pop_front(matches);
    }
    list_free(matches);

    List(size_t) *accept_states = set_get_list(dfa->accept_states);
    while(list_size(accept_states)) {
        dfa_add_accept_state(flat_dfa, list_get_front(accept_states));
        list_pop_front(accept_states);
    }
    list_free(accept_states);

    return flat_dfa;
}

void _flat_dfa_print(const char *dfa_str) {
    size_t sz = ((const size_t *)((void*) dfa_str))[0];
    size_t num_transitions = ((const size_t *)((void*) dfa_str))[1];
    size_t last = (num_transitions << sz) + (sizeof(size_t) << 1);
    for(size_t i = 0; i < last; ++i) {
        if(i % 16 == 0)
            printf("\n");
        printf("'\\x");
        printf("%02hhX", dfa_str[i]);
        printf("'");
        if(i != last - 1)
            printf(", ");
    }
    printf("\n");
}

_flat_dfa_fns_t _flat_fns = {
    &_flat_dfa_run, &_flat_dfa_run_greedy,
    &_flat_dfa_run_greedy_iterator, &_flat_dfa_add_transition,
    &_flat_dfa_remove_transition, &_flat_dfa_add_accepting_state,
    &_flat_dfa_remove_accepting_state,
    &_flat_dfa_free
};

_flat_dfa_t* _flat_dfa_deserialize(const char *ptr) {
    _flat_dfa_t *new_dfa = (_flat_dfa_t*) malloc(sizeof(_flat_dfa_t));
    assert(((size_t*) ptr)[0] == _flat_dfa_offset_constant);
    new_dfa->state_size = ((size_t*) ptr)[1];
    new_dfa->transition = (char*) malloc(new_dfa->state_size << _flat_dfa_offset_constant);
    size_t offset = (sizeof(size_t) << 1);
    for(size_t i = 0; i < new_dfa->state_size << _flat_dfa_offset_constant; ++i) {
        new_dfa->transition[i] = ((char*) ptr)[offset + i];
    }
    new_dfa->fns = &_flat_fns;
    return new_dfa;
}

/* Returns a new dfa_t */
_flat_dfa_t* _flat_dfa_new(size_t state_size) {
    _flat_dfa_t *new_dfa = (_flat_dfa_t*) malloc(sizeof(_flat_dfa_t));
    new_dfa->state_size = state_size;
    // printf("Size of transition: %zu\n", state_size << _flat_dfa_offset_constant);
    new_dfa->transition = (char*) malloc(state_size << _flat_dfa_offset_constant);
    for(size_t i = 0; i < state_size << _flat_dfa_offset_constant; ++i) {
        new_dfa->transition[i] = '\xff';
    }
    new_dfa->fns = &_flat_fns;
    return new_dfa;
}