#ifndef DFA_C
#define DFA_C

#include "../util/list.h"

#ifndef call 
#define call(context, arg)      ((context)->call((context), (arg)))
#endif

/**
 * Definitions for `fdfa_t' and `dfa_t' are listed here. 
 * 
 * `dfa_t' can be converted to `fdfa_t' using `dfa_to_fdfa'.
 * 
 * Usage:
 * ----- DFA -----
 *   Dfa(state_type, transition_type) *dfa = 
 *     dfa_new(Map({ST, TT}, ST), ST, List(ST));
 *
 *     >> where dfa(1,2,3)  ->  (1) dfa transitions
 *                              (2) begin state
 *                              (3) list of accept states
 *     - dfa_run(dfa, Iterator(TT))   ->   size_t
 *     - dfa_to_fdfa(dfa)             ->   fdfa_t
 * 
 * ----- FDFA -----
 *   fdfa_t *fdfa = dfa_to_fdfa(dfa);
 *     - call(fdfa, str)              ->   size_t
 */

// Defines a transition.
#define define_transition(state_type, transition_type) \
    struct _##state_type##_##transition_type##_transition_ { \
        state_type state; \
        transition_type transition; \
    }; \
    typedef struct _##state_type##_##transition_type##_transition_ _##state_type##_##transition_type##_transition_t

// Defines a dfa with state_type (st) and transition_type (tt)
#define define_dfa(st, tt) \
    define_transition(st, tt); \
    size_t _##st##_##tt##_dfa_new() { \
    \
    }

define_dfa(int, char);

#endif