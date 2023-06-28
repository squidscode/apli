#ifndef NFA_H
#define NFA_H

#include "dfa.h"

/**
 * An NFA is simply a DFA that uses the nfa_transition. 
 *
 * Usage:
 * ----- NFA -----
 *   Nfa(state_type, transition_type) *nfa = nfa_new(state_type, transition_type, ST);
 *     - nfa_add_transition(nfa, ST, TT, ST)         ->   void
 *     - nfa_add_epsilon_transition(nfa, ST, ST)     ->   void
 *     - nfa_add_kleen_star(nfa, ST, ST)             ->   void
 *     - nfa_remove_transition(nfa, ST, TT)          ->   size_t
 *     - nfa_remove_epsilon_transition(nfa, ST, ST)  ->   size_t
 *     - nfa_remove_kleen_star(nfa, ST, ST)          ->   size_t
 *     - nfa_add_accept_state(nfa, ST)               ->   void
 *     - nfa_remove_accept_state(nfa, ST)            ->   size_t
 *     - nfa_free(nfa)                               ->   void
 *     - nfa_to_dfa(nfa)                             ->   dfa_t
 */

#ifndef UNTYPED_NFA_FNS
#define UNTYPED_NFA_FNS
#define nfa_new(state_type, transition_type, start_state)
#define nfa_add_transition(nfa, from, transition, to)
#define nfa_add_epsilon_transition(nfa, from, to)
#define nfa_add_kleen_star_transition(nfa, from, to)
#define nfa_remove_transition(nfa, from, transition)
#define nfa_remove_epsilon_transition(nfa, from, to)
#define nfa_remove_kleen_star_transition(nfa, from, to)
#define nfa_add_accept_state(nfa, state)
#define nfa_remove_accept_state(nfa, state)
#define nfa_free(nfa)
#define nfa_to_dfa(nfa)
#endif

#define define_nfa_transition_type(type) \
    enum nfa_transition_type {NONE, EPSILON, STAR}; \
    struct _##type##_nfa_transition_ { \
        nfa_transition_type transition_type; \
        type val; \
    }; \
    typedef _##type##_nfa_transition_ _##type##_nfa_transition_t

#define define_nfa(st, tt) \
    define_dfa(st, tt); \
    define_nfa_transition_type(tt); \
    typedef Vector(st)* st_vector_ptr; \
    define_list(_##tt##_nfa_transition_t); \
    define_dfa(st_vector_ptr, _##tt##_nfa_transition_t);
    
define_vector(int);
define_list(char);
define_nfa(int, char);

#endif