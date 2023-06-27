#ifndef DFA_C
#define DFA_C

#include <assert.h>
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
 *   Dfa(state_type, transition_type) *dfa = dfa_new(ST);
 *     - dfa_run(dfa, Iterator(TT))       ->   size_t
 *     - dfa_add_transition(ST, TT, ST)   ->   void
 *     - dfa_remove_transition(ST, TT)    ->   size_t
 *     - dfa_add_accepting_state(ST)      ->   void
 *     - dfa_remove_accepting_state(ST)   ->   size_t
 *     - dfa_free(dfa)                    ->   void
 *     - dfa_to_fdfa(dfa)                 ->   fdfa_t   ** locks the DFA, mutating methods throw errors **
 * 
 * ----- FDFA (An immutable DFA) -----
 *   fdfa_t *fdfa = dfa_to_fdfa(dfa);
 *     - call(fdfa, const char*)          ->   size_t
 */

// Defines a transition.
#define define_transition(state_type, transition_type) \
    struct _##state_type##_##transition_type##_transition_ { \
        state_type state; \
        transition_type transition; \
    }; \
    typedef struct _##state_type##_##transition_type##_transition_ _##state_type##_##transition_type##_transition_t; \
    struct _##state_type##_##transition_type##_state_data_ { \
        unsigned int accept_flag : 1; \
    }; \
    typedef struct _##state_type##_##transition_type##_state_data_ _##state_type##_##transition_type##_state_data_t;


#define init_dfa_types(st, tt) \
    /* Type definitions: */ \
    define_transition(st, tt); \
    define_map(_##st##_##tt##_transition_t, st); \
    define_map(st, _##st##_##tt##_state_data_t); \
    define_list(st); \
    define_list(tt)

// TODO virtual table for dfa_t
// Defines a dfa with state_type (st) and transition_type (tt)
#define define_dfa(st, tt) \
    struct _##st##_##tt##_dfa_; \
    /* Virtual table for mutable DFA functions */ \
    struct _##st##_##tt##_dfa_fns_ { \
        size_t (*run)(struct _##st##_##tt##_dfa_*, Iterator(tt)); \
        void (*add_transition)(struct _##st##_##tt##_dfa_*, st, tt, st); \
        size_t (*remove_transition)(struct _##st##_##tt##_dfa_*, st, tt); \
        void (*add_accepting_state)(struct _##st##_##tt##_dfa_*, st); \
        size_t (*remove_accepting_state)(struct _##st##_##tt##_dfa_*, st); \
        void (*free)(struct _##st##_##tt##_dfa_*); \
        void (*dfa_to_fdfa)(struct _##st##_##tt##_dfa_*); \
    }; \
    typedef struct _##st##_##tt##_dfa_fns_ _##st##_##tt##_dfa_fns_t; \
    \
    struct _##st##_##tt##_dfa_ { \
        Map(_##st##_##tt##_transition_t, st) *transition_map; \
        st begin_state; \
        Map(st, _##st##_##tt##_state_data_t) *accept_state; \
        unsigned int is_locked : 1; \
    }; \
    typedef struct _##st##_##tt##_dfa_ _##st##_##tt##_dfa_t; \
    \
    void _##st##_##tt##_assert_is_not_locked(_##st##_##tt##_dfa_t *dfa) { \
        if (dfa->is_locked)
            assert(0 == "");
    }
    void _##st##_##tt##_dfa_run(_##st##_##tt##_dfa_t *dfa, Iterator(tt) *transition_iter) { \
        _##st##_##tt##_assert_is_not_locked(dfa);
        st current_state = dfa->begin_state; \
        _##st##_##tt##_transition_t trans; \
        while(transition_iter != NULL) { \
            trans.state = current_state; trans.transition = iter_val(transition_iter); \
            if (map_count(dfa->transition_map, trans)) \
                current_state = map_at(dfa->transition_map, )
            transition_iter = iter_next(transition_iter); \
        } \
    } \
    \
    _##st##_##tt##_dfa_t _##st##_##tt##_dfa_new() { \
        \
    } \

// An `fdfa_t' mimics a function closure (since the `call' function pointer takes, as an argument,
// the `fdfa_t' that it is called on). Global state data that we want to capture when creating a 
// function closure can be placed inside the fdfa_t struct, and the `call' function pointer can access
// that data by accessing its `fdfa_t' argument.
struct _fdfa_ {
    /* TODO add data we need to capture here! */
    size_t (*call)(struct _fdfa_*, const char*);
};
typedef struct _fdfa_ fdfa_t;

init_dfa_types(int, char);
define_dfa(int, char);

#endif