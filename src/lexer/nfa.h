#ifndef NFA_H
#define NFA_H

#include "dfa.h"
#include "../util/set.h"

/**
 * An NFA is simply a DFA that uses the nfa_transition. Internally, each transition is from a 
 * `set of states' to another `set of states' through an `nfa_transition'. NFA construction is done
 * as new transitions are added to the graph (so the nfa_to_dfa call is, virtually, an O(1) call).
 *
 * Usage:
 * ----- NFA -----
 *   Nfa(state_type, transition_type) *nfa = nfa_new(state_type, transition_type, ST);
 *     - nfa_add_transition(nfa, ST, TT, ST)         ->   void
 *     - nfa_add_epsilon_transition(nfa, ST, ST)     ->   void
 *     - nfa_add_alphabet(nfa, ST, ST)               ->   void
 *     - nfa_remove_transition(nfa, ST, TT, ST)      ->   size_t
 *     - nfa_remove_epsilon_transition(nfa, ST, ST)  ->   size_t
 *     - nfa_remove_alphabet(nfa, ST, ST)            ->   size_t
 *     - nfa_add_accept_state(nfa, ST)               ->   void
 *     - nfa_remove_accept_state(nfa, ST)            ->   size_t
 *     - nfa_free(nfa)                               ->   void
 *     - nfa_to_dfa(nfa)                             ->   dfa_t*
 *       ^^^ automatically frees the nfa ^^^
 */

#ifndef UNTYPED_NFA_FNS
#define UNTYPED_NFA_FNS
#define Nfa(state_type, transition_type)
#define nfa_new(state_type, transition_type, start_state)
#define nfa_add_transition(nfa, from, transition, to)
#define nfa_add_epsilon_transition(nfa, from, to)
#define nfa_add_alphabet_transition(nfa, from, to)
#define nfa_remove_transition(nfa, from, transition)
#define nfa_remove_epsilon_transition(nfa, from, to)
#define nfa_remove_alphabet_transition(nfa, from, to)
#define nfa_add_accept_state(nfa, state)
#define nfa_remove_accept_state(nfa, state)
#define nfa_free(nfa)
#define nfa_to_dfa(nfa)
#endif

typedef enum _nfa_transition_type_ {NONE, ALL} nfa_transition_type;

#define define_nfa_transition_type(st, tt) \
    struct _##tt##_nfa_transition_ { \
        nfa_transition_type transition_type; \
        tt val; \
    }; \
    typedef struct _##tt##_nfa_transition_ _##tt##_nfa_transition_t; \
    \
    struct _##st##_##tt##_nfa_transition_ { \
        st state; \
        _##tt##_nfa_transition_t transition; \
    }; \
    typedef struct _##st##_##tt##_nfa_transition_ _##st##_##tt##_nfa_transition_t; \

#define init_nfa(st, tt) \
    define_set(st); \
    define_list(tt); \
    define_nfa_transition_type(st, tt); \
    typedef Set(st)* st##_set_ptr_t; \
    define_list(_##tt##_nfa_transition_t); \
    define_map(_##st##_##tt##_nfa_transition_t, st##_set_ptr_t); \
    define_map(st, st##_set_ptr_t); \
    define_dfa(st##_set_ptr_t, tt); \
    define_set(tt)

#define define_nfa(st, tt) \
    struct _##st##_##tt##_nfa_; \
    /* Virtual table for mutable DFA functions */ \
    struct _##st##_##tt##_nfa_fns_ { \
        void (*add_transition)(struct _##st##_##tt##_nfa_*, st, tt, st); \
        void (*add_epsilon_transition)(struct _##st##_##tt##_nfa_*, st, st); \
        void (*add_alphabet_transition)(struct _##st##_##tt##_nfa_*, st, st); \
        size_t (*remove_transition)(struct _##st##_##tt##_nfa_*, st, tt, st); \
        size_t (*remove_epsilon_transition)(struct _##st##_##tt##_nfa_*, st, st); \
        size_t (*remove_alphabet_transition)(struct _##st##_##tt##_nfa_*, st, st); \
        void (*add_accept_state)(struct _##st##_##tt##_nfa_*, st); \
        size_t (*remove_accept_state)(struct _##st##_##tt##_nfa_*, st); \
        Dfa(st##_set_ptr_t, tt)* (*nfa_to_dfa)(struct _##st##_##tt##_nfa_*, Set(tt)*); \
        void (*free)(struct _##st##_##tt##_nfa_*); \
    }; \
    \
    struct _##st##_##tt##_nfa_ { \
        _##st##_##tt##_nfa_transition_t tmp_transition; \
        _##st##_##tt##_nfa_transition_t tmp_all_transition; \
        Map(_##st##_##tt##_nfa_transition_t, st##_set_ptr_t) *transition_map; \
        st begin_state; \
        Set(st) *accept_states; \
        Set(st) *all_states; \
        Map(st, st##_set_ptr_t) *epsilon_map; \
        struct _##st##_##tt##_nfa_fns_ *fns; \
    }; \
    typedef struct _##st##_##tt##_nfa_ _##st##_##tt##_nfa_t; \
    \
    /* Add a new_set to the map if the node isn't already mapped to something. */ \
    void _##st##_##tt##_nfa_add_epsilon_node(Map(st, st##_set_ptr_t) *map, st node) { \
        if(0 == map_count(map, node)) \
            map_insert(map, node, set_new(st)); \
    } \
    \
    /* Adds a new set to the transition map at the transition if it doesn't exist */ \
    void _##st##_##tt##_nfa_add_transition_node(Map(_##st##_##tt##_nfa_transition_t, st##_set_ptr_t) *transition_map, \
        _##st##_##tt##_nfa_transition_t transition) { \
        if(0 == map_count(transition_map, transition)) \
            map_insert(transition_map, transition, set_new(st)); \
    } \
    \
    /* Adds a transition to the transition map. */ \
    void _##st##_##tt##_nfa_add_transition(_##st##_##tt##_nfa_t *nfa, st from, tt transition, st to) { \
        set_insert(nfa->all_states, from); set_insert(nfa->all_states, to); \
        nfa->tmp_transition.state = from; nfa->tmp_transition.transition.transition_type = NONE; \
        nfa->tmp_transition.transition.val = transition; \
        _##st##_##tt##_nfa_add_transition_node(nfa->transition_map, nfa->tmp_transition); \
        set_insert(map_at(nfa->transition_map, nfa->tmp_transition), to); \
    } \
    \
    /* Adds an epsilon transition to the nfa. */ \
    void _##st##_##tt##_nfa_add_epsilon_transition(_##st##_##tt##_nfa_t *nfa, st from, st to) { \
        set_insert(nfa->all_states, from); set_insert(nfa->all_states, to); \
        _##st##_##tt##_nfa_add_epsilon_node(nfa->epsilon_map, from); \
        _##st##_##tt##_nfa_add_epsilon_node(nfa->epsilon_map, to); \
        set_insert(map_at(nfa->epsilon_map, from), to); \
    } \
    \
    /* Adds an alphabet transition. */ \
    void _##st##_##tt##_nfa_add_alphabet_transition(_##st##_##tt##_nfa_t *nfa, st from, st to) { \
        set_insert(nfa->all_states, from); set_insert(nfa->all_states, to); \
        nfa->tmp_all_transition.state = from; nfa->tmp_all_transition.transition.transition_type = ALL; \
        _##st##_##tt##_nfa_add_transition_node(nfa->transition_map, nfa->tmp_all_transition); \
        set_insert(map_at(nfa->transition_map, nfa->tmp_all_transition), to); \
    } \
    \
    /* Removes the transition from the transition map. */ \
    size_t _##st##_##tt##_nfa_remove_transition(_##st##_##tt##_nfa_t *nfa, st from, tt transition, st to) { \
        nfa->tmp_transition.state = from; nfa->tmp_transition.transition.transition_type = NONE; \
        nfa->tmp_transition.transition.val = transition; \
        if(0 == map_count(nfa->transition_map, nfa->tmp_transition)) \
            return 0; \
        else \
            return set_erase(map_at(nfa->transition_map, nfa->tmp_transition), to); \
    } \
    \
    size_t _##st##_##tt##_nfa_remove_epsilon_transition(_##st##_##tt##_nfa_t *nfa, st from, st to) { \
        if(0 == map_count(nfa->epsilon_map, from)) \
            return 0; \
        else \
            return set_erase(map_at(nfa->epsilon_map, from), to); \
    } \
    \
    size_t _##st##_##tt##_nfa_remove_alphabet_transition(_##st##_##tt##_nfa_t *nfa, st from, st to) { \
        nfa->tmp_all_transition.state = from; nfa->tmp_all_transition.transition.transition_type = ALL; \
        if(0 == map_count(nfa->transition_map, nfa->tmp_all_transition)) \
            return 0; \
        else \
            return set_erase(map_at(nfa->transition_map, nfa->tmp_all_transition), to); \
    } \
    \
    void _##st##_##tt##_nfa_add_accept_state(_##st##_##tt##_nfa_t *nfa, st state) { \
        return set_insert(nfa->accept_states, state); \
    } \
    \
    size_t _##st##_##tt##_nfa_remove_accept_state(_##st##_##tt##_nfa_t *nfa, st state) { \
        return set_erase(nfa->accept_states, state); \
    } \
    \
    void _##st##_##tt##_fill_epsilon_reachable_map(_##st##_##tt##_nfa_t *nfa, Map(st, st##_set_ptr_t) *epsilon_reachable_map) { \
        
    } \
    \
    Dfa(st##_set_ptr_t, tt)* _##st##_##tt##_nfa_to_dfa(_##st##_##tt##_nfa_t *nfa, Set(tt) *alphabet_set) { \
        Map(st, st##_set_ptr_t) *epsilon_reachable_map = map_new(st, st##_set_ptr_t); \
        _##st##_##tt##_fill_epsilon_reachable_map(nfa, epsilon_reachable_map); \
        Dfa(st##_set_ptr_t, tt) *new_dfa = dfa_new(st##_set_ptr_t, tt, map_at(epsilon_reachable_map, nfa->begin_state)); \
        map_free(epsilon_reachable_map); \
        nfa_free(nfa); \
    } \
    \
    void _##st##_##tt##_nfa_free(_##st##_##tt##_nfa_t *nfa) { \
        map_free(nfa->transition_map); \
        set_free(nfa->accept_states); \
        map_free(nfa->epsilon_map); \
        free(nfa); \
    } \
    \
    struct _##st##_##tt##_nfa_fns_ st##_##tt##_nfa_fns = { \
        &_##st##_##tt##_nfa_add_transition, &_##st##_##tt##_nfa_add_epsilon_transition, \
        &_##st##_##tt##_nfa_add_alphabet_transition, &_##st##_##tt##_nfa_remove_transition, \
        &_##st##_##tt##_nfa_remove_epsilon_transition, &_##st##_##tt##_nfa_remove_alphabet_transition, \
        &_##st##_##tt##_nfa_add_accept_state, &_##st##_##tt##_nfa_remove_accept_state, \
        &_##st##_##tt##_nfa_to_dfa, &_##st##_##tt##_nfa_free \
    }; \
    \
    _##st##_##tt##_nfa_t* _##st##_##tt##_nfa_new(st begin_state) { \
        _##st##_##tt##_nfa_t* new_nfa = (_##st##_##tt##_nfa_t*) malloc(sizeof(_##st##_##tt##_nfa_t)); \
        new_nfa->transition_map = map_new(_##st##_##tt##_nfa_transition_t, st##_set_ptr_t); \
        new_nfa->begin_state = begin_state; \
        new_nfa->accept_states = set_new(st); \
        new_nfa->epsilon_map = map_new(st, st##_set_ptr_t); \
        map_insert(new_nfa->epsilon_map, begin_state, set_new(st)); \
        new_nfa->fns = &st##_##tt##_nfa_fns; \
        return new_nfa; \
    } \


#endif