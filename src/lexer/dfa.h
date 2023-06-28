#ifndef DFA_H
#define DFA_H

#include <assert.h>
#include <string.h>
#include "../util/list.h"
#include "../util/vector.h"
#include "../util/map.h"

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
 * Usage:
 * ----- DFA (A mutable DFA) -----
 *   Dfa(state_type, transition_type) *dfa = dfa_new(state_type, transition_type, ST);
 *     - dfa_run(dfa, Iterator(TT))            ->   size_t
 *     - dfa_add_transition(dfa, ST, TT, ST)   ->   void
 *     - dfa_remove_transition(dfa, ST, TT)    ->   size_t
 *     - dfa_add_accept_state(dfa, ST)         ->   void
 *     - dfa_remove_accept_state(dfa, ST)      ->   size_t
 *     - dfa_free(dfa)                         ->   void
 *     - dfa_to_fdfa(dfa)                      ->   fdfa_t
 *       ^^^^^^^^^^^^^^^^ locks the dfa
 * 
 * ----- FDFA (An immutable DFA) -----
 *   fdfa_t *fdfa = dfa_to_fdfa(dfa);
 *     - call(fdfa, const char*)          ->   size_t
 */

#ifndef UNTYPED_DFA_FNS
#define UNTYPED_DFA_FNS
#define Dfa(st, tt)                                 _##st##_##tt##_dfa_t
#define Fdfa(st, tt)                                _##st##_##tt##_fdfa_t
#define dfa_run(dfa, iter)                          ((dfa)->fns->run((dfa), (iter)))
#define dfa_add_transition(dfa, from, trans, to)    ((dfa)->fns->add_transition((dfa), (from), (trans), (to)))
#define dfa_remove_transition(dfa, state, trans)    ((dfa)->fns->remove_transition((dfa), (state), (trans)))
#define dfa_add_accept_state(dfa, state)            ((dfa)->fns->add_accept_state((dfa), (state)))
#define dfa_remove_accept_state(dfa, state)         ((dfa)->fns->remove_accept_state((dfa), (state)))
#define dfa_free(dfa)                               ((dfa)->fns->free((dfa)))
#define dfa_to_fdfa(dfa)                            ((dfa)->fns->dfa_to_fdfa((dfa)))
#define fdfa_free(fdfa)                             ((fdfa)->free((fdfa)))
#define dfa_new(st, tt, begin_state)                (_##st##_##tt##_dfa_new(begin_state))
#endif

// Defines a transition.
#define define_transition(state_type, transition_type) \
    struct _##state_type##_##transition_type##_transition_ { \
        state_type state; \
        transition_type transition; \
    }; \
    typedef struct _##state_type##_##transition_type##_transition_ _##state_type##_##transition_type##_transition_t; \
    \
    struct _##state_type##_##transition_type##_state_data_ { \
        unsigned int accept_flag : 1; \
    }; \
    typedef struct _##state_type##_##transition_type##_state_data_ _##state_type##_##transition_type##_state_data_t; 


#define init_dfa_types(st, tt) \
    /* Type definitions: */ \
    define_transition(st, tt); \
    define_map(_##st##_##tt##_transition_t, st); \
    define_map(st, _##st##_##tt##_state_data_t) \

// Defines a dfa with state_type (st) and transition_type (tt)
#define define_dfa(st, tt) \
    init_dfa_types(st, tt); \
    struct _##st##_##tt##_dfa_; \
    define_fdfa(st, tt); \
    \
    /* Virtual table for mutable DFA functions */ \
    struct _##st##_##tt##_dfa_fns_ { \
        size_t (*run)(struct _##st##_##tt##_dfa_*, Iterator(tt)*); \
        void (*add_transition)(struct _##st##_##tt##_dfa_*, st, tt, st); \
        size_t (*remove_transition)(struct _##st##_##tt##_dfa_*, st, tt); \
        void (*add_accept_state)(struct _##st##_##tt##_dfa_*, st); \
        size_t (*remove_accept_state)(struct _##st##_##tt##_dfa_*, st); \
        void (*free)(struct _##st##_##tt##_dfa_*); \
        _##st##_##tt##_fdfa_t* (*dfa_to_fdfa)(struct _##st##_##tt##_dfa_*); \
    }; \
    typedef struct _##st##_##tt##_dfa_fns_ _##st##_##tt##_dfa_fns_t; \
    \
    /* A struct that represents a dfa. */ \
    struct _##st##_##tt##_dfa_ { \
        _##st##_##tt##_transition_t tmp_trans; \
        Map(_##st##_##tt##_transition_t, st) *transition_map; \
        st begin_state; \
        Map(st, _##st##_##tt##_state_data_t) *accept_states; \
        unsigned int is_locked : 1; \
        _##st##_##tt##_dfa_fns_t *fns; \
    }; \
    typedef struct _##st##_##tt##_dfa_ _##st##_##tt##_dfa_t; \
    \
    /* Throws an assert error if the dfa is locked. */ \
    void _##st##_##tt##_assert_is_not_locked(_##st##_##tt##_dfa_t *dfa) { \
        if (dfa->is_locked) \
            assert(0 == "The dfa is locked and cannot be modified."); \
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
        return map_count(dfa->accept_states, current_state); \
    } \
    \
    void _##st##_##tt##_dfa_add_transition(_##st##_##tt##_dfa_t *dfa, st from, tt transition, st to) { \
        _##st##_##tt##_assert_is_not_locked(dfa); \
        dfa->tmp_trans.state = from; dfa->tmp_trans.transition = transition; \
        /* printf("Transition (%i, %c) hash: %llu\n", dfa->tmp_trans.state, dfa->tmp_trans.transition, dfa->transition_map->hash(dfa->tmp_trans)); */ \
        map_insert(dfa->transition_map, dfa->tmp_trans, to); \
    } \
    \
    size_t _##st##_##tt##_dfa_remove_transition(_##st##_##tt##_dfa_t *dfa, st state, tt transition) { \
        _##st##_##tt##_assert_is_not_locked(dfa); \
        dfa->tmp_trans.state = state; dfa->tmp_trans.transition = transition; \
        return map_erase(dfa->transition_map, dfa->tmp_trans); \
    } \
    \
    void _##st##_##tt##_dfa_add_accepting_state( _##st##_##tt##_dfa_t *dfa, st state) { \
        _##st##_##tt##_assert_is_not_locked(dfa); \
        _##st##_##tt##_state_data_t sd = {1}; \
        map_insert(dfa->accept_states, state, sd); \
    } \
    \
    size_t _##st##_##tt##_dfa_remove_accepting_state(_##st##_##tt##_dfa_t *dfa, st state) { \
        _##st##_##tt##_assert_is_not_locked(dfa); \
        return map_erase(dfa->accept_states, state); \
    } \
    \
    void _##st##_##tt##_dfa_free(_##st##_##tt##_dfa_t *dfa) { \
        map_free(dfa->transition_map); \
        map_free(dfa->accept_states); \
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
        new_fdfa->free = &_##st##_##tt##_fdfa_free; \
        dfa->is_locked = 1; \
        return new_fdfa; \
    } \
    \
    _##st##_##tt##_dfa_fns_t _##st##_##tt##_fns = { \
        &_##st##_##tt##_dfa_run, &_##st##_##tt##_dfa_add_transition, \
        &_##st##_##tt##_dfa_remove_transition, &_##st##_##tt##_dfa_add_accepting_state, \
        &_##st##_##tt##_dfa_remove_accepting_state, &_##st##_##tt##_dfa_free, \
        &_##st##_##tt##_dfa_to_fdfa \
    }; \
    \
    /* Returns a new dfa_t */ \
    _##st##_##tt##_dfa_t* _##st##_##tt##_dfa_new(st begin_state) { \
        _##st##_##tt##_dfa_t *new_dfa = (_##st##_##tt##_dfa_t*) malloc(sizeof(_##st##_##tt##_dfa_t)); \
        new_dfa->transition_map = map_new(_##st##_##tt##_transition_t, st); \
        new_dfa->begin_state = begin_state; \
        new_dfa->accept_states = map_new(st, _##st##_##tt##_state_data_t); \
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
        void (*free)(struct _##st##_##tt##_fdfa_*); \
    }; \
    typedef struct _##st##_##tt##_fdfa_ _##st##_##tt##_fdfa_t;    

#endif