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

#define Nfa(state_type, transition_type)                        _##state_type##_##transition_type##_nfa_t
#define nfa_new(state_type, transition_type, start_state)       (_##state_type##_##transition_type##_nfa_new(start_state))
#define nfa_add_transition(nfa, from, transition, to)           ((nfa)->fns->add_transition((nfa), (from), (transition), (to)))
#define nfa_add_epsilon_transition(nfa, from, to)               ((nfa)->fns->add_epsilon_transition((nfa), (from), (to)))
#define nfa_add_alphabet_transition(nfa, from, to)              ((nfa)->fns->add_alphabet_transition((nfa), (from), (to)))
#define nfa_remove_transition(nfa, from, transition, to)        ((nfa)->fns->remove_transition((nfa), (from), (transition), (to)))
#define nfa_remove_epsilon_transition(nfa, from, to)            ((nfa)->fns->remove_epsilon_transition((nfa), (from), (to)))
#define nfa_remove_alphabet_transition(nfa, from, to)           ((nfa)->fns->remove_alphabet_transition((nfa), (from), (to)))
#define nfa_add_accept_state(nfa, state)                        ((nfa)->fns->add_accept_state((nfa), (state)))
#define nfa_remove_accept_state(nfa, state)                     ((nfa)->fns->remove_accept_state((nfa), (state)))
#define nfa_to_dfa(nfa, alphabet)                               ((nfa)->fns->nfa_to_dfa((nfa), (alphabet)))
#define nfa_free(nfa)                                           ((nfa)->fns->destroy((nfa)))

/**
 * Powerset transformation algorithm:
 *   (1) Construct a [st -> Set(st)] map for epsilon reachable states.
 *   (2) Construct a [st -> [tt -> List(st)]] map for all possible transitions from a given state
 *   (3) Run BFS from the ERS(root) and construct a DFA via the following rule:
 *     - For each transition [state_1 -> {transition, state_2}], a transition from:
 *                   ERS_1  -----transition----> ERS_2 exists, where
 *       ERS_1 contains state_1 AND ERS_2 contains state_2, and 
 *       iff state_i in ERS_1 and state_i -----transition----> state_j exists, then state_j is
 *       in ERS_2.
 *     ** Run DFS/BFS from ERS(root) to construct this! **
 * 
 */

typedef enum _nfa_transition_type_ {NONE, ALL} nfa_transition_type;
// TODO refactor iterator to list mutation.

#define define_nfa_transition_type(st, tt) \
    struct _##tt##_nfa_transition_ { \
        nfa_transition_type transition_type; \
        tt val; \
    }; \
    typedef struct _##tt##_nfa_transition_ _##tt##_nfa_transition_t; \
    typedef Set(st)* st##_set_ptr_t; \
    \
    define_list(_##tt##_nfa_transition_t); \
    define_set(_##tt##_nfa_transition_t); \
    define_map(_##tt##_nfa_transition_t, st##_set_ptr_t); \
    typedef Map(_##tt##_nfa_transition_t, st##_set_ptr_t)* _##st##_##tt##_nfa_transition_map_t

#define init_nfa(st, tt) \
    define_set(st); \
    define_set_hash(st); \
    define_nfa_transition_type(st, tt); \
    define_list(st##_set_ptr_t); \
    define_map(st, _##st##_##tt##_nfa_transition_map_t); \
    define_map(st, st##_set_ptr_t); \
    define_set(st##_set_ptr_t); \
    init_dfa_types(st##_set_ptr_t, tt); \
    define_dfa(st##_set_ptr_t, tt); \
    define_vector(tt)

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
        Dfa(st##_set_ptr_t, tt)* (*nfa_to_dfa)(struct _##st##_##tt##_nfa_*, Vector(tt)*); \
        void (*destroy)(struct _##st##_##tt##_nfa_*); \
    }; \
    \
    struct _##st##_##tt##_nfa_ { \
        _##tt##_nfa_transition_t tmp_transition; \
        _##tt##_nfa_transition_t tmp_all_transition; \
        Map(st, _##st##_##tt##_nfa_transition_map_t) *transition_map; \
        st begin_state; \
        Set(st) *accept_states; \
        Set(st) *all_states; \
        Map(st, st##_set_ptr_t) *epsilon_map; \
        List(st##_set_ptr_t) *free_state_set_list; \
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
    void _##st##_##tt##_nfa_add_transition_node(Map(st, _##st##_##tt##_nfa_transition_map_t) *transition_map, \
        st state, _##tt##_nfa_transition_t transition) { \
        if(0 == map_count(transition_map, state)) { \
            map_insert(transition_map, state, map_new(_##tt##_nfa_transition_t, st##_set_ptr_t)); \
        } \
        if(0 == map_count(map_at(transition_map, state), transition)) { \
            map_insert(map_at(transition_map, state), transition, set_new(st)); \
        } \
    } \
    \
    /* Adds a transition to the transition map. */ \
    void _##st##_##tt##_nfa_add_transition(_##st##_##tt##_nfa_t *nfa, st from, tt transition, st to) { \
        set_insert(nfa->all_states, from); set_insert(nfa->all_states, to); \
        nfa->tmp_transition.transition_type = NONE; \
        nfa->tmp_transition.val = transition; \
        _##st##_##tt##_nfa_add_transition_node(nfa->transition_map, from, nfa->tmp_transition); \
        set_insert(map_at(map_at(nfa->transition_map, from), nfa->tmp_transition), to); \
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
        nfa->tmp_all_transition.transition_type = ALL; \
        _##st##_##tt##_nfa_add_transition_node(nfa->transition_map, from, nfa->tmp_all_transition); \
        set_insert(map_at(map_at(nfa->transition_map, from), nfa->tmp_all_transition), to); \
    } \
    \
    /* Removes the transition from the transition map. */ \
    size_t _##st##_##tt##_nfa_remove_transition(_##st##_##tt##_nfa_t *nfa, st from, tt transition, st to) { \
        nfa->tmp_transition.transition_type = NONE; \
        nfa->tmp_transition.val = transition; \
        if(0 == map_count(nfa->transition_map, from) \
            || 0 == map_count(map_at(nfa->transition_map, from), nfa->tmp_transition)) \
            return 0; \
        else \
            return set_erase(map_at(map_at(nfa->transition_map, from), nfa->tmp_transition), to); \
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
        nfa->tmp_all_transition.transition_type = ALL; \
        if(0 == map_count(nfa->transition_map, from) \
            || 0 == map_count(map_at(nfa->transition_map, from), nfa->tmp_transition)) \
            return 0; \
        else \
            return set_erase(map_at(map_at(nfa->transition_map, from), nfa->tmp_all_transition), to); \
    } \
    \
    void _##st##_##tt##_nfa_add_accept_state(_##st##_##tt##_nfa_t *nfa, st state) { \
        set_insert(nfa->accept_states, state); \
    } \
    \
    size_t _##st##_##tt##_nfa_remove_accept_state(_##st##_##tt##_nfa_t *nfa, st state) { \
        return set_erase(nfa->accept_states, state); \
    } \
    \
    /* Creates the ERS map */ \
    void _##st##_##tt##_fill_epsilon_reachable_map(_##st##_##tt##_nfa_t *nfa, Map(st, st##_set_ptr_t) *epsilon_reachable_map) { \
        List(st) *list_of_states = set_get_list(nfa->all_states); \
        Iterator(st) *iterator_of_states = list_get_iterator(list_of_states); \
        while(iterator_of_states != NULL) { \
            st root = iter_val(iterator_of_states); \
            Set(st) *seen = set_new(st); \
            list_push_back(nfa->free_state_set_list, seen); /* this set belongs to the nfa, and should be freed with the nfa. */ \
            List(st) *stk = list_new(st); list_push_back(stk, root); \
            while(0 < list_size(stk)) { \
                st nxt = list_get_first(stk); list_pop_front(stk); \
                if(0 == set_count(seen, nxt)) \
                    set_insert(seen, nxt); \
                else \
                    continue; \
                if(0 == map_count(nfa->epsilon_map, nxt)) continue; \
                Set(st) *set_of_adj = map_at(nfa->epsilon_map, nxt); \
                List(st) *list_of_adj = set_get_list(set_of_adj); \
                Iterator(st) *iter_of_adj = list_get_iterator(list_of_adj); \
                while(NULL != iter_of_adj) { \
                    list_push_back(stk, iter_val(iter_of_adj)); \
                    iter_of_adj = iter_next(iter_of_adj); \
                } \
                list_free(list_of_adj); \
            } \
            map_insert(epsilon_reachable_map, root, seen); \
            list_free(stk); \
            iterator_of_states = iter_next(iterator_of_states); \
        } \
        list_free(list_of_states); \
    } \
    \
    void _##st##_##tt##_add_all_transitions_to_nfa(_##st##_##tt##_nfa_t *nfa, Vector(tt) *alphabet_set, st from, Set(st) *to) { \
        List(st) *to_states = set_get_list(to); \
        while(0 < list_size(to_states)) { \
            size_t alphabet_size = vector_size(alphabet_set); \
            for(size_t i = 0; i < alphabet_size; ++i) { \
                nfa_add_transition(nfa, from, vector_get(alphabet_set, i), list_get_front(to_states)); \
            } \
            list_pop_front(to_states); \
        } \
        list_free(to_states); \
    } \
    \
    void _##st##_##tt##_replace_all_transitions_with_alphabet_set_transitions(_##st##_##tt##_nfa_t *nfa, Vector(tt) *alphabet_set) { \
        List(_##st##__##st##_##tt##_nfa_transition_map_t_map_match_t) *state_transition_matches = map_get_list(nfa->transition_map); \
        while(0 < list_size(state_transition_matches)) { \
            List(__##tt##_nfa_transition_t_##st##_set_ptr_t_map_match_t) *transition_state_matches = map_get_list(map_at(nfa->transition_map, \
                list_get_front(state_transition_matches).key)); \
            while(0 < list_size(transition_state_matches)) { \
                if(ALL == list_get_front(transition_state_matches).key.transition_type) \
                    _##st##_##tt##_add_all_transitions_to_nfa(nfa, alphabet_set, list_get_front(state_transition_matches).key, \
                        list_get_front(transition_state_matches).value); \
                list_pop_front(transition_state_matches); \
            } \
            list_free(transition_state_matches); \
            list_pop_front(state_transition_matches); \
        } \
        list_free(state_transition_matches); \
    } \
    \
    Set(st)* _##st##_##tt##_compute_transition_set(_##st##_##tt##_nfa_t *nfa, Map(st, st##_set_ptr_t) *epsilon_reachable_map, \
        Set(st) *next_set, _##tt##_nfa_transition_t transition) { \
        Set(st) *transition_set = set_new(st); \
        list_push_back(nfa->free_state_set_list, transition_set); /* the nfa owns this set! */ \
        List(st) *state_list = set_get_list(next_set); \
        while(0 < list_size(state_list)) { \
            st current_state = list_get_front(state_list); \
            if(0 == map_count(nfa->transition_map, current_state) \
                || 0 == map_count(map_at(nfa->transition_map, current_state), transition)) { \
                list_pop_front(state_list); \
                continue; \
            } \
            Set(st) *direct_transition_states = map_at(map_at(nfa->transition_map, current_state), transition); \
            List(st) *dts_list = set_get_list(direct_transition_states); \
            while(0 < list_size(dts_list)) { \
                set_union(transition_set, map_at(epsilon_reachable_map, list_get_front(dts_list))); \
                list_pop_front(dts_list); \
            } \
            list_free(dts_list); \
            list_pop_front(state_list); \
        } \
        list_free(state_list); \
        return transition_set; \
    } \
    \
    void _##st##_##tt##_load_dfa_with_transition(Dfa(st##_set_ptr_t, tt) *new_dfa, \
        Set(st) *from, _##tt##_nfa_transition_t transition, Set(st) *to) { \
        if(NONE == transition.transition_type) { \
            /* construct_int_char_transition_string(from, transition.val, to); TODO delete this!! */ \
            dfa_add_transition(new_dfa, from, transition.val, to); \
        } \
    } \
    \
    void _##st##_##tt##_update_accept_status_in_dfa(Dfa(st##_set_ptr_t, tt) *new_dfa, _##st##_##tt##_nfa_t *nfa, \
        Set(st) *states) { \
        List(st) *list_of_states = set_get_list(states); \
        while(0 < list_size(list_of_states)) { \
            if(1 == set_count(nfa->accept_states, list_get_front(list_of_states))) { \
                dfa_add_accept_state(new_dfa, states); \
                list_free(list_of_states); \
                return; \
            } \
            list_pop_front(list_of_states); \
        } \
        list_free(list_of_states); \
    } \
    \
    void _##st##_##tt##_process_next_state_set(Dfa(st##_set_ptr_t, tt) *new_dfa, \
        _##st##_##tt##_nfa_t *nfa, Map(st, st##_set_ptr_t) *epsilon_reachable_map, List(st##_set_ptr_t)* state_queue, \
        Set(st) *next_set) { \
        List(st) *state_list = set_get_list(next_set); \
        _##st##_##tt##_update_accept_status_in_dfa(new_dfa, nfa, next_set); \
        Set(_##tt##_nfa_transition_t) *seen_transitions = set_new(_##tt##_nfa_transition_t); \
        while(0 < list_size(state_list)) { \
            if(0 == map_count(nfa->transition_map, list_get_front(state_list))) { \
                list_pop_front(state_list); \
                continue; \
            } \
            List(__##tt##_nfa_transition_t_##st##_set_ptr_t_map_match_t) *transition_list \
                = map_get_list(map_at(nfa->transition_map, list_get_front(state_list))); \
            while(0 < list_size(transition_list)) { \
                if(1 == set_count(seen_transitions, list_get_front(transition_list).key)) { \
                    list_pop_front(transition_list); \
                    continue; \
                } \
                set_insert(seen_transitions, list_get_front(transition_list).key); \
                Set(st) *transition_state = _##st##_##tt##_compute_transition_set(nfa, \
                    epsilon_reachable_map, next_set, list_get_front(transition_list).key); \
                _##st##_##tt##_load_dfa_with_transition(new_dfa, next_set, list_get_front(transition_list).key, \
                    transition_state); \
                _##st##_##tt##_update_accept_status_in_dfa(new_dfa, nfa, transition_state); \
                list_push_back(state_queue, transition_state); \
                list_pop_front(transition_list); \
            } \
            list_pop_front(state_list); \
            list_free(transition_list); \
        } \
        set_free(seen_transitions); \
        list_free(state_list); \
    } \
    \
    void _##st##_##tt##_construct_dfa_with_transitions_and_epsilon_reachable_map(Dfa(st##_set_ptr_t, tt) *new_dfa, \
        _##st##_##tt##_nfa_t *nfa, Map(st, st##_set_ptr_t) *epsilon_reachable_map) { \
        List(st##_set_ptr_t) *state_queue = list_new(st##_set_ptr_t); \
        Set(st##_set_ptr_t) *seen = set_new(st##_set_ptr_t); \
        set_set_hash(seen, &_##st##_set_collection_hash); \
        set_set_value_equals(seen, &_##st##_set_equals_); \
        list_push_back(state_queue, map_at(epsilon_reachable_map, nfa->begin_state)); \
        while(0 < list_size(state_queue)) { \
            Set(st) *next_set = list_get_front(state_queue); \
            if(0 == set_count(seen, next_set)) { \
                _##st##_##tt##_process_next_state_set(new_dfa, nfa, epsilon_reachable_map, state_queue, next_set); \
                set_insert(seen, next_set); \
            } \
            list_pop_front(state_queue); \
        } \
        list_free(state_queue); \
        set_free(seen); \
    } \
    \
    /* Hash and equals overrides for the transition. */ \
    size_t _##st##_##tt##_transition_hash_fn(_##st##_set_ptr_t_##tt##_transition_t transition) { \
        return _##st##_set_collection_hash(transition.state); \
    } \
    size_t _##st##_##tt##_transition_equals_fn(_##st##_set_ptr_t_##tt##_transition_t transition1, \
        _##st##_set_ptr_t_##tt##_transition_t transition2) { \
        return transition1.transition == transition2.transition \
            && _##st##_set_equals_(transition1.state, transition2.state); \
    } \
    \
    void _##st##_##tt##_override_dfa_tranisition_map_equals(Dfa(st##_set_ptr_t, tt) *new_dfa) {\
        map_set_hash(new_dfa->transition_map, &_##st##_##tt##_transition_hash_fn); \
        map_set_key_eq(new_dfa->transition_map, &_##st##_##tt##_transition_equals_fn); \
        set_set_hash(new_dfa->accept_states, &_##st##_set_collection_hash); \
        set_set_value_equals(new_dfa->accept_states, &_##st##_set_equals_); \
    } \
    \
    /* Converts an NFA into a DFA using the powerset algorithm. This mutates the current NFA, so this algorithm can \
       only be called *once*. */ \
    Dfa(st##_set_ptr_t, tt)* _##st##_##tt##_nfa_to_dfa(_##st##_##tt##_nfa_t *nfa, Vector(tt) *alphabet_set) { \
        Map(st, st##_set_ptr_t) *epsilon_reachable_map = map_new(st, st##_set_ptr_t); \
        _##st##_##tt##_fill_epsilon_reachable_map(nfa, epsilon_reachable_map); \
        _##st##_##tt##_replace_all_transitions_with_alphabet_set_transitions(nfa, alphabet_set); \
        Dfa(st##_set_ptr_t, tt) *new_dfa = dfa_new(st##_set_ptr_t, tt, map_at(epsilon_reachable_map, nfa->begin_state)); \
        _##st##_##tt##_override_dfa_tranisition_map_equals(new_dfa); \
        _##st##_##tt##_construct_dfa_with_transitions_and_epsilon_reachable_map(new_dfa, nfa, epsilon_reachable_map); \
        map_free(epsilon_reachable_map); \
        return new_dfa; \
    } \
    \
    void _##st##_##tt##_nfa_free(_##st##_##tt##_nfa_t *nfa) { \
        /* TODO this attempt at freeing the nfa is INCOMPLETE! */ \
        /* (1) Free the transition map */ \
        List(_##st##__##st##_##tt##_nfa_transition_map_t_map_match_t) *state_transition_matches = map_get_list(nfa->transition_map); \
        while(0 < list_size(state_transition_matches)) { \
            List(__##tt##_nfa_transition_t_##st##_set_ptr_t_map_match_t) *transition_state_matches = map_get_list(map_at(nfa->transition_map, \
                list_get_front(state_transition_matches).key)); \
            while(0 < list_size(transition_state_matches)) { \
                set_free(list_get_front(transition_state_matches).value); \
                list_pop_front(transition_state_matches); \
            } \
            list_free(transition_state_matches); \
            map_free(map_at(nfa->transition_map, list_get_front(state_transition_matches).key)); \
            list_pop_front(state_transition_matches); \
        } \
        list_free(state_transition_matches); \
        map_free(nfa->transition_map); \
        \
        /* (2) Free the accept_states set */ \
        set_free(nfa->accept_states); \
        \
        /* (3) Free the all_states set */ \
        set_free(nfa->all_states); \
        \
        /* (4) Free the epsilon_map map */ \
        List(_##st##_##st##_set_ptr_t_map_match_t) *epsilon_transitions = map_get_list(nfa->epsilon_map); \
        while(0 < list_size(epsilon_transitions)) { \
            set_free(list_get_front(epsilon_transitions).value); \
            list_pop_front(epsilon_transitions); \
        } \
        list_free(epsilon_transitions); \
        map_free(nfa->epsilon_map); \
        \
        while(0 < list_size(nfa->free_state_set_list)) { \
            set_free(list_get_front(nfa->free_state_set_list)); \
            list_pop_front(nfa->free_state_set_list); \
        } \
        list_free(nfa->free_state_set_list); \
        \
        /* (5) Free the original nfa */ \
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
        new_nfa->transition_map = map_new(st, _##st##_##tt##_nfa_transition_map_t); \
        new_nfa->begin_state = begin_state; \
        new_nfa->all_states = set_new(st); \
        new_nfa->accept_states = set_new(st); \
        new_nfa->epsilon_map = map_new(st, st##_set_ptr_t); \
        new_nfa->free_state_set_list = list_new(st##_set_ptr_t); \
        map_insert(new_nfa->epsilon_map, begin_state, set_new(st)); \
        new_nfa->fns = &st##_##tt##_nfa_fns; \
        return new_nfa; \
    } \


#endif