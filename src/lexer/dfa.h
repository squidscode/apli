#ifndef DFA_H
#define DFA_H

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../util/list.h"
#include "../util/vector.h"
#include "../util/map.h"
#include "../util/set.h"

#ifndef call 
#define call(context, arg)      ((context)->call((context), (arg)))
#endif

/**
 * Definitions for `fdfa_t' and `dfa_t' are listed here. 
 * 
 * `dfa_t' can be converted to `fdfa_t' using `dfa_to_fdfa'. After it is converted to an fdfa, 
 * the given dfa is locked and can no longer be mutated (because it is being used by the fdfa).
 * An `assert' error will occur if the user tries to mutate the dfa after an fdfa is generated.
 *
 * DFA's that accept a specific language can be encoded as functions that 
 * take in a string (const char*) and return `1` if it accepts or `0` if 
 * it rejects. In order to create a list of tokens from a given string, a 
 * binary-search-like algorithm with a DFA function that accepts if the string
 * **contains** the given token can be used. 
 * 
 * In order to avoid confusion, I will be separating the `function' representation
 * of a DFA from an interal (map-based) representation of a DFA
 *   - An `fdfa_t' is a dfa represented as a function closure (or a C-like representation of a 
 *     function closure)
 *     - TYPE : [context] st.
 *       - call(context, const char*)   ->   size_t (indicating true[1] / false[0])
 *   - A `dfa_t' is a dfa represented through a directed graph. The internal representation
 *     of a `dfa_t' isn't important, because a function that converts a `dfa_t' to a
 *     `fdfa_t' must be supplied.
 * 
 * Usage:
 * ----- DFA (A mutable DFA) -----
 *   Dfa(state_type, transition_type) *dfa = dfa_new(state_type, transition_type, ST);
 *     - dfa_run(dfa, Iterator(TT))            ->   size_t
 *     - dfa_add_transition(dfa, ST, TT, ST)   ->   void
 *     - dfa_remove_transition(dfa, ST, TT)    ->   size_t
 *     - dfa_add_accept_state(dfa, ST)         ->   void
 *     - dfa_remove_accept_state(dfa, ST)      ->   size_t
 *     - dfa_free(dfa)                         ->   void
 *     - dfa_compress(dfa)                     ->   Dfa(size_t, transition_type)
 *     - dfa_to_fdfa(dfa)                      ->   fdfa_t *
 *       ^^^^^^^^^^^^^^^^ locks the dfa
 * 
 * ----- FDFA (An immutable DFA) -----
 *   fdfa_t *fdfa = dfa_to_fdfa(dfa);
 *     - call(fdfa, const char*)          ->   size_t
 */

#define Dfa(st, tt)                                 _##st##_##tt##_dfa_t
#define Fdfa(st, tt)                                _##st##_##tt##_fdfa_t
#define dfa_run(dfa, iter)                          ((dfa)->fns->run((dfa), (iter)))
#define dfa_run_greedy(dfa, ptr, sz)                ((dfa)->fns->run_greedy((dfa), (ptr), (sz)))
#define dfa_run_greedy_iter(dfa, iter)              ((dfa)->fns->run_greedy_iter((dfa), (iter)))
#define dfa_add_transition(dfa, from, trans, to)    ((dfa)->fns->add_transition((dfa), (from), (trans), (to)))
#define dfa_remove_transition(dfa, state, trans)    ((dfa)->fns->remove_transition((dfa), (state), (trans)))
#define dfa_add_accept_state(dfa, state)            ((dfa)->fns->add_accept_state((dfa), (state)))
#define dfa_remove_accept_state(dfa, state)         ((dfa)->fns->remove_accept_state((dfa), (state)))
#define dfa_free(dfa)                               ((dfa)->fns->destroy((dfa)))
#define dfa_compress(dfa)                           ((dfa)->fns->compress((dfa)))
#define dfa_to_fdfa(dfa)                            ((dfa)->fns->dfa_to_fdfa((dfa)))
#define fdfa_free(fdfa)                             ((fdfa)->destroy((fdfa)))
#define dfa_new(st, tt, begin_state)                (_##st##_##tt##_dfa_new(begin_state))

// Defines a transition.
#define define_transition(state_type, transition_type) \
    struct _##state_type##_##transition_type##_transition_ { \
        state_type state; \
        transition_type transition; \
    } __attribute__((packed)); \
    typedef struct _##state_type##_##transition_type##_transition_ _##state_type##_##transition_type##_transition_t


#define init_dfa_types(st, tt) \
    /* Type definitions: */ \
    define_transition(st, tt); \
    define_map(st, size_t); \
    define_map(_##st##_##tt##_transition_t, st)

// Defines a dfa with state_type (st) and transition_type (tt)
#define define_dfa(st, tt) \
    struct _##st##_##tt##_dfa_; \
    define_fdfa(st, tt); \
    \
    /* Virtual table for mutable DFA functions */ \
    struct _##st##_##tt##_dfa_fns_ { \
        size_t (*run)(struct _##st##_##tt##_dfa_*, Iterator(tt)*); \
        size_t (*run_greedy)(struct _##st##_##tt##_dfa_*, const tt*, size_t); \
        size_t (*run_greedy_iter)(struct _##st##_##tt##_dfa_*, Iterator(tt)*); \
        void (*add_transition)(struct _##st##_##tt##_dfa_*, st, tt, st); \
        size_t (*remove_transition)(struct _##st##_##tt##_dfa_*, st, tt); \
        void (*add_accept_state)(struct _##st##_##tt##_dfa_*, st); \
        size_t (*remove_accept_state)(struct _##st##_##tt##_dfa_*, st); \
        struct _size_t_##tt##_dfa_* (*compress)(struct _##st##_##tt##_dfa_*); \
        void (*destroy)(struct _##st##_##tt##_dfa_*); \
        _##st##_##tt##_fdfa_t* (*dfa_to_fdfa)(struct _##st##_##tt##_dfa_*); \
    }; \
    typedef struct _##st##_##tt##_dfa_fns_ _##st##_##tt##_dfa_fns_t; \
    \
    /* A struct that represents a dfa. */ \
    struct _##st##_##tt##_dfa_ { \
        _##st##_##tt##_transition_t tmp_trans; \
        Map(_##st##_##tt##_transition_t, st) *transition_map; \
        st begin_state; \
        Set(st) *accept_states; \
        unsigned int is_locked : 1; \
        _##st##_##tt##_dfa_fns_t *fns; \
    }; \
    typedef struct _##st##_##tt##_dfa_ _##st##_##tt##_dfa_t; \
    _##st##_##tt##_dfa_t* _##st##_##tt##_dfa_new(st); \
    \
    /* Throws an assert error if the dfa is locked. */ \
    inline void _##st##_##tt##_assert_is_not_locked(_##st##_##tt##_dfa_t *dfa) { \
        if (dfa->is_locked) \
            assert(0 == "The dfa is locked and cannot be modified."); \
    } \
    \
    /* Runs the dfa with the given transition iterator. */ \
    size_t _##st##_##tt##_dfa_run_greedy(_##st##_##tt##_dfa_t *dfa, const tt *ptr, size_t ptr_sz) { \
        st current_state = dfa->begin_state; \
        size_t offset = 0UL; \
        size_t max_right_bound = ~0UL; \
        size_t state = 0; \
        while(offset <= ptr_sz) { \
            dfa->tmp_trans.state = current_state; dfa->tmp_trans.transition = ptr[offset]; \
            /* printf("OFFSET: %zu, STATE: %zu\n", offset, current_state); */ \
            if(current_state == dfa->begin_state && max_right_bound != ~0UL) { \
                /* printf("RETURNING\n"); */ \
                return max_right_bound;\
            } else if(set_count(dfa->accept_states, current_state)) { \
                /* printf("State is accepting!\n"); */ \
                max_right_bound = offset; \
            } \
            \
            if (map_count(dfa->transition_map, dfa->tmp_trans)) { \
                current_state = map_at(dfa->transition_map, dfa->tmp_trans); \
            } else { \
                current_state = dfa->begin_state; \
            } \
            offset += 1; \
        } \
        return max_right_bound; \
    } \
    \
    /* Runs the dfa with the given transition iterator. */ \
    size_t _##st##_##tt##_dfa_run_greedy_iterator(_##st##_##tt##_dfa_t *dfa, Iterator(tt) *transition_iter) { \
        st current_state = dfa->begin_state; \
        Iterator(tt) *iter_ptr = transition_iter; \
        size_t offset = 0UL; \
        size_t max_right_bound = ~0UL; \
        size_t state = 0; \
        while(iter_ptr != NULL) { \
            dfa->tmp_trans.state = current_state; dfa->tmp_trans.transition = iter_val(transition_iter); \
            if(current_state == dfa->begin_state && max_right_bound != ~0UL) { \
                return max_right_bound;\
            } else if (set_count(dfa->accept_states, current_state)) { \
                max_right_bound = offset; \
            } \
            \
            if (map_count(dfa->transition_map, dfa->tmp_trans)) { \
                current_state = map_at(dfa->transition_map, dfa->tmp_trans); \
            } else { \
                current_state = dfa->begin_state; \
            } \
            transition_iter = iter_next(transition_iter); \
            offset += 1; \
        } \
        return max_right_bound; \
    } \
    \
    /* Runs the dfa with the given transition iterator. */ \
    size_t _##st##_##tt##_dfa_run(_##st##_##tt##_dfa_t *dfa, Iterator(tt) *transition_iter) { \
        st current_state = dfa->begin_state; \
        while(transition_iter != NULL) { \
            dfa->tmp_trans.state = current_state; dfa->tmp_trans.transition = iter_val(transition_iter); \
            /* printf("Transition (%i, %c) hash: %llu\n", dfa->tmp_trans.state, dfa->tmp_trans.transition, dfa->transition_map->hash(dfa->tmp_trans)); */ \
            if (map_count(dfa->transition_map, dfa->tmp_trans)) \
                current_state = map_at(dfa->transition_map, dfa->tmp_trans); \
            else \
                return 0; \
            transition_iter = iter_next(transition_iter); \
        } \
        return set_count(dfa->accept_states, current_state); \
    } \
    \
    void _##st##_##tt##_dfa_add_transition(_##st##_##tt##_dfa_t *dfa, st from, tt transition, st to) { \
        /* _##st##_##tt##_assert_is_not_locked(dfa); */ \
        dfa->tmp_trans.state = from; dfa->tmp_trans.transition = transition; \
        /* printf("Transition (%i, %c) hash: %llu\n", dfa->tmp_trans.state, dfa->tmp_trans.transition, dfa->transition_map->hash(dfa->tmp_trans)); */ \
        map_insert(dfa->transition_map, dfa->tmp_trans, to); \
    } \
    \
    size_t _##st##_##tt##_dfa_remove_transition(_##st##_##tt##_dfa_t *dfa, st state, tt transition) { \
        /* _##st##_##tt##_assert_is_not_locked(dfa); */ \
        dfa->tmp_trans.state = state; dfa->tmp_trans.transition = transition; \
        return map_erase(dfa->transition_map, dfa->tmp_trans); \
    } \
    \
    void _##st##_##tt##_dfa_add_accepting_state( _##st##_##tt##_dfa_t *dfa, st state) { \
        /* _##st##_##tt##_assert_is_not_locked(dfa); */ \
        set_insert(dfa->accept_states, state); \
    } \
    \
    size_t _##st##_##tt##_dfa_remove_accepting_state(_##st##_##tt##_dfa_t *dfa, st state) { \
        /* _##st##_##tt##_assert_is_not_locked(dfa); */ \
        return set_erase(dfa->accept_states, state); \
    } \
    \
    void _size_t_##tt##_dfa_add_transition(Dfa(size_t, tt)*, size_t, tt, size_t); \
    void _size_t_##tt##_dfa_add_accepting_state(Dfa(size_t, tt)*, size_t); \
    \
    Dfa(size_t, tt)* _##st##_##tt##_dfa_compress(struct _##st##_##tt##_dfa_ *dfa) { \
        Dfa(size_t, tt)* new_dfa = dfa_new(size_t, tt, 0UL); \
        Map(st, size_t)* state_map = map_new(st, size_t); \
        map_set_hash(state_map, dfa->accept_states->hash); \
        map_set_key_eq(state_map, dfa->accept_states->value_equals); \
        map_insert(state_map, dfa->begin_state, 0UL); \
        \
        /* Oh no, this type is long... */ \
        __##st##_##tt##_transition_t_##st##_map_match_t_list_t *list_of_transitions = map_get_list(dfa->transition_map); \
        __##st##_##tt##_transition_t_##st##_map_match_t_list_node_t *list_iterator = list_get_iterator(list_of_transitions); \
        \
        size_t counter = 1UL; \
        while(iter_is_not_null(list_iterator)) { \
            __##st##_##tt##_transition_t_##st##_map_match_t nxt_match = iter_val(list_iterator); \
            if(0 == map_count(state_map, nxt_match.key.state)) { \
                /*printf("compress %zu -> %zu\n", nxt_match.key.state, counter);*/ \
                map_insert(state_map, nxt_match.key.state, counter); \
                ++counter; \
            } \
            if(0 == map_count(state_map, nxt_match.value)) { \
                /*printf("compress %zu -> %zu\n", nxt_match.value, counter);*/ \
                map_insert(state_map, nxt_match.value, counter); \
                ++counter; \
            } \
            list_iterator = iter_next(list_iterator); \
        } \
        \
        list_iterator = list_get_iterator(list_of_transitions); \
        while(iter_is_not_null(list_iterator)) { \
            __##st##_##tt##_transition_t_##st##_map_match_t nxt_match = iter_val(list_iterator); \
            /*printf("compress transition: %zu, %c -> %zu\n", map_at(state_map, nxt_match.key.state), nxt_match.key.transition, map_at(state_map, nxt_match.value));*/ \
            _size_t_##tt##_dfa_add_transition(new_dfa, map_at(state_map, nxt_match.key.state), nxt_match.key.transition, map_at(state_map, nxt_match.value)); \
            list_iterator = iter_next(list_iterator); \
        } \
        \
        list_free(list_of_transitions); \
        \
        List(st) *list_of_accept_states = set_get_list(dfa->accept_states); \
        while(list_size(list_of_accept_states)) { \
            st nxt = list_get_front(list_of_accept_states); \
            /*printf("accepting state: %zu\n", map_at(state_map, nxt));*/\
            _size_t_##tt##_dfa_add_accepting_state(new_dfa, map_at(state_map, nxt)); \
            list_pop_front(list_of_accept_states); \
        } \
        \
        map_free(state_map); \
        list_free(list_of_accept_states); \
        \
        return new_dfa; \
    } \
    \
    void _##st##_##tt##_dfa_free(_##st##_##tt##_dfa_t *dfa) { \
        map_free(dfa->transition_map); \
        set_free(dfa->accept_states); \
        free(dfa); \
    } \
    \
    size_t _##st##_##tt##_fdfa_call(_##st##_##tt##_fdfa_t *fdfa, Iterator(tt) *iter) { \
        return dfa_run(fdfa->dfa, iter); \
    } \
    \
    void _##st##_##tt##_fdfa_free(_##st##_##tt##_fdfa_t *fdfa) { \
        map_free(fdfa->dfa); \
        free(fdfa); \
    } \
    \
    _##st##_##tt##_fdfa_t* _##st##_##tt##_dfa_to_fdfa(_##st##_##tt##_dfa_t *dfa) { \
        _##st##_##tt##_fdfa_t *new_fdfa = (_##st##_##tt##_fdfa_t*) malloc(sizeof(_##st##_##tt##_fdfa_t)); \
        new_fdfa->dfa = dfa; \
        new_fdfa->call = &_##st##_##tt##_fdfa_call; \
        new_fdfa->destroy = &_##st##_##tt##_fdfa_free; \
        dfa->is_locked = 1; \
        return new_fdfa; \
    } \
    \
    _##st##_##tt##_dfa_fns_t _##st##_##tt##_fns = { \
        &_##st##_##tt##_dfa_run, &_##st##_##tt##_dfa_run_greedy, \
        &_##st##_##tt##_dfa_run_greedy_iterator, &_##st##_##tt##_dfa_add_transition, \
        &_##st##_##tt##_dfa_remove_transition, &_##st##_##tt##_dfa_add_accepting_state, \
        &_##st##_##tt##_dfa_remove_accepting_state, &_##st##_##tt##_dfa_compress, \
        &_##st##_##tt##_dfa_free, &_##st##_##tt##_dfa_to_fdfa \
    }; \
    \
    /* Returns a new dfa_t */ \
    _##st##_##tt##_dfa_t* _##st##_##tt##_dfa_new(st begin_state) { \
        _##st##_##tt##_dfa_t *new_dfa = (_##st##_##tt##_dfa_t*) malloc(sizeof(_##st##_##tt##_dfa_t)); \
        new_dfa->transition_map = map_new(_##st##_##tt##_transition_t, st); \
        new_dfa->begin_state = begin_state; \
        new_dfa->accept_states = set_new(st); \
        new_dfa->is_locked = 0; \
        new_dfa->fns = &_##st##_##tt##_fns; \
        return new_dfa; \
    } \
    


/**
 * An `fdfa_t' mimics a function closure (since the `call' function pointer takes, as an argument,
 * the `fdfa_t' that it is called on). Global state data that we want to capture when creating a 
 * function closure can be placed inside the fdfa_t struct, and the `call' function pointer can access
 * that data by accessing its `fdfa_t' argument.
 */
#define define_fdfa(st, tt) \
    struct _##st##_##tt##_fdfa_ { \
        struct _##st##_##tt##_dfa_ *dfa; \
        size_t (*call)(struct _##st##_##tt##_fdfa_*, Iterator(tt)*); \
        void (*destroy)(struct _##st##_##tt##_fdfa_*); \
    }; \
    typedef struct _##st##_##tt##_fdfa_ _##st##_##tt##_fdfa_t;    

#endif