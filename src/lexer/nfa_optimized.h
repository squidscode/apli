#ifndef NFA_OPT_H
#define NFA_OPT_H

#include "flat_dfa.c"
#include "../util/set.h"
#include "../util/bitset.h"

/**
 * An NFA is simply a DFA that uses the nfa_transition. Internally, each transition is from a 
 * `set of states' to another `set of states' through an `nfa_transition'. NFA construction is done
 * as new transitions are added to the graph (so the nfa_to_dfa call is, virtually, an O(1) call).
 *
 * Usage:
 * ----- NFA -----
 *   Nfa(state_type, transition_type) *nfa = nfa_new(state_type, transition_type, ST);
 *     - nfa_add_transition(nfa, ST, char, ST)         ->   void
 *     - nfa_add_epsilon_transition(nfa, ST, ST)     ->   void
 *     - nfa_add_alphabet(nfa, ST, ST)               ->   void
 *     - nfa_remove_transition(nfa, ST, char, ST)      ->   size_t
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
 *   (1) Construct a [st -> Set(size_t)] map for epsilon reachable states.
 *   (2) Construct a [st -> [char -> List(size_t)]] map for all possible transitions from a given state
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

struct _char_nfa_transition_ {
    nfa_transition_type transition_type;
    char val;
};
typedef struct _char_nfa_transition_ _char_nfa_transition_t;
typedef _bitset_t* size_t_set_ptr_t;

define_list(_char_nfa_transition_t);
define_set(_char_nfa_transition_t);
define_map(_char_nfa_transition_t, size_t_set_ptr_t);
typedef Map(_char_nfa_transition_t, size_t_set_ptr_t)* _size_t_char_nfa_transition_map_t;

define_list(size_t_set_ptr_t);
define_map(size_t, _size_t_char_nfa_transition_map_t);
define_map(size_t, size_t_set_ptr_t);
define_set(size_t_set_ptr_t);
init_dfa_types(size_t_set_ptr_t, char);
typedef struct _size_t_char_dfa_ _size_t_char_dfa_t;
_size_t_char_dfa_t* _size_t_char_dfa_new(size_t);
define_dfa(size_t_set_ptr_t, char);
define_vector(char)

struct _size_t_char_nfa_;
/* Virtual table for mutable DFA functions */
struct _size_t_char_nfa_fns_ {
    void (*add_transition)(struct _size_t_char_nfa_*, size_t, char, size_t);
    void (*add_epsilon_transition)(struct _size_t_char_nfa_*, size_t, size_t);
    void (*add_alphabet_transition)(struct _size_t_char_nfa_*, size_t, size_t);
    size_t (*remove_transition)(struct _size_t_char_nfa_*, size_t, char, size_t);
    size_t (*remove_epsilon_transition)(struct _size_t_char_nfa_*, size_t, size_t);
    size_t (*remove_alphabet_transition)(struct _size_t_char_nfa_*, size_t, size_t);
    void (*add_accept_state)(struct _size_t_char_nfa_*, size_t);
    size_t (*remove_accept_state)(struct _size_t_char_nfa_*, size_t);
    Dfa(size_t_set_ptr_t, char)* (*nfa_to_dfa)(struct _size_t_char_nfa_*, Vector(char)*);
    void (*destroy)(struct _size_t_char_nfa_*);
};

struct _size_t_char_nfa_ {
    _char_nfa_transition_t tmp_transition;
    _char_nfa_transition_t tmp_all_transition;
    Map(size_t, _size_t_char_nfa_transition_map_t) *transition_map;
    size_t begin_state;
    size_t_set_ptr_t accept_states;
    size_t_set_ptr_t all_states;
    Map(size_t, size_t_set_ptr_t) *epsilon_map;
    List(size_t_set_ptr_t) *free_state_set_list;
    struct _size_t_char_nfa_fns_ *fns;
};
typedef struct _size_t_char_nfa_ _size_t_char_nfa_t;

/* Add a new_set to the map if the node isn't already mapped to something. */
void _size_t_char_nfa_add_epsilon_node(Map(size_t, size_t_set_ptr_t) *map, size_t node) {
    if(0 == map_count(map, node))
        map_insert(map, node, bitset_new());
}

/* Adds a new set to the transition map at the transition if it doesn't exist */
void _size_t_char_nfa_add_transition_node(Map(size_t, _size_t_char_nfa_transition_map_t) *transition_map,
    size_t state, _char_nfa_transition_t transition) {
    if(0 == map_count(transition_map, state)) {
        map_insert(transition_map, state, map_new(_char_nfa_transition_t, size_t_set_ptr_t));
    }
    if(0 == map_count(map_at(transition_map, state), transition)) {
        map_insert(map_at(transition_map, state), transition, bitset_new());
    }
}

/* Adds a transition to the transition map. */
void _size_t_char_nfa_add_transition(_size_t_char_nfa_t *nfa, size_t from, char transition, size_t to) {
    set_insert(nfa->all_states, from); set_insert(nfa->all_states, to);
    nfa->tmp_transition.transition_type = NONE;
    nfa->tmp_transition.val = transition;
    _size_t_char_nfa_add_transition_node(nfa->transition_map, from, nfa->tmp_transition);
    set_insert(map_at(map_at(nfa->transition_map, from), nfa->tmp_transition), to);
}

/* Adds an epsilon transition to the nfa. */
void _size_t_char_nfa_add_epsilon_transition(_size_t_char_nfa_t *nfa, size_t from, size_t to) {
    set_insert(nfa->all_states, from); set_insert(nfa->all_states, to);
    _size_t_char_nfa_add_epsilon_node(nfa->epsilon_map, from);
    _size_t_char_nfa_add_epsilon_node(nfa->epsilon_map, to);
    set_insert(map_at(nfa->epsilon_map, from), to);
}

/* Adds an alphabet transition. */
void _size_t_char_nfa_add_alphabet_transition(_size_t_char_nfa_t *nfa, size_t from, size_t to) {
    set_insert(nfa->all_states, from); set_insert(nfa->all_states, to);
    nfa->tmp_all_transition.transition_type = ALL;
    _size_t_char_nfa_add_transition_node(nfa->transition_map, from, nfa->tmp_all_transition);
    set_insert(map_at(map_at(nfa->transition_map, from), nfa->tmp_all_transition), to);
}

/* Removes the transition from the transition map. */
size_t _size_t_char_nfa_remove_transition(_size_t_char_nfa_t *nfa, size_t from, char transition, size_t to) {
    nfa->tmp_transition.transition_type = NONE;
    nfa->tmp_transition.val = transition;
    if(0 == map_count(nfa->transition_map, from)
        || 0 == map_count(map_at(nfa->transition_map, from), nfa->tmp_transition))
        return 0;
    else
        return set_erase(map_at(map_at(nfa->transition_map, from), nfa->tmp_transition), to);
}

size_t _size_t_char_nfa_remove_epsilon_transition(_size_t_char_nfa_t *nfa, size_t from, size_t to) {
    if(0 == map_count(nfa->epsilon_map, from))
        return 0;
    else
        return set_erase(map_at(nfa->epsilon_map, from), to);
}

size_t _size_t_char_nfa_remove_alphabet_transition(_size_t_char_nfa_t *nfa, size_t from, size_t to) {
    nfa->tmp_all_transition.transition_type = ALL;
    if(0 == map_count(nfa->transition_map, from)
        || 0 == map_count(map_at(nfa->transition_map, from), nfa->tmp_transition))
        return 0;
    else
        return set_erase(map_at(map_at(nfa->transition_map, from), nfa->tmp_all_transition), to);
}

void _size_t_char_nfa_add_accept_state(_size_t_char_nfa_t *nfa, size_t state) {
    set_insert(nfa->accept_states, state);
}

size_t _size_t_char_nfa_remove_accept_state(_size_t_char_nfa_t *nfa, size_t state) {
    return set_erase(nfa->accept_states, state);
}

/* Creates the ERS map */
void _size_t_char_fill_epsilon_reachable_map(_size_t_char_nfa_t *nfa, Map(size_t, size_t_set_ptr_t) *epsilon_reachable_map) {
    List(size_t) *list_of_states = set_get_list(nfa->all_states);
    Iterator(size_t) *iterator_of_states = list_get_iterator(list_of_states);
    while(iterator_of_states != NULL) {
        size_t root = iter_val(iterator_of_states);
        _bitset_t* seen = bitset_new();
        list_push_back(nfa->free_state_set_list, seen); /* this set belongs to the nfa, and should be freed with the nfa. */
        List(size_t) *stk = list_new(size_t); list_push_back(stk, root);
        while(0 < list_size(stk)) {
            size_t nxt = list_get_first(stk); list_pop_front(stk);
            if(0 == set_count(seen, nxt))
                set_insert(seen, nxt);
            else
                continue;
            if(0 == map_count(nfa->epsilon_map, nxt)) continue;
            _bitset_t* set_of_adj = map_at(nfa->epsilon_map, nxt);
            List(size_t) *list_of_adj = set_get_list(set_of_adj);
            Iterator(size_t) *iter_of_adj = list_get_iterator(list_of_adj);
            while(NULL != iter_of_adj) {
                list_push_back(stk, iter_val(iter_of_adj));
                iter_of_adj = iter_next(iter_of_adj);
            }
            list_free(list_of_adj);
        }
        map_insert(epsilon_reachable_map, root, seen);
        list_free(stk);
        iterator_of_states = iter_next(iterator_of_states);
    }
    list_free(list_of_states);
}

void _size_t_char_add_all_transitions_to_nfa(_size_t_char_nfa_t *nfa, Vector(char) *alphabet_set, size_t from, _bitset_t* to) {
    List(size_t) *to_states = set_get_list(to);
    while(0 < list_size(to_states)) {
        size_t alphabet_size = vector_size(alphabet_set);
        for(size_t i = 0; i < alphabet_size; ++i) {
            nfa_add_transition(nfa, from, vector_get(alphabet_set, i), list_get_front(to_states));
        }
        list_pop_front(to_states);
    }
    list_free(to_states);
}

void _size_t_char_replace_all_transitions_with_alphabet_set_transitions(_size_t_char_nfa_t *nfa, Vector(char) *alphabet_set) {
    List(_size_t__size_t_char_nfa_transition_map_t_map_match_t) *state_transition_matches = map_get_list(nfa->transition_map);
    while(0 < list_size(state_transition_matches)) {
        List(__char_nfa_transition_t_size_t_set_ptr_t_map_match_t) *transition_state_matches = map_get_list(map_at(nfa->transition_map,
            list_get_front(state_transition_matches).key));
        while(0 < list_size(transition_state_matches)) {
            if(ALL == list_get_front(transition_state_matches).key.transition_type)
                _size_t_char_add_all_transitions_to_nfa(nfa, alphabet_set, list_get_front(state_transition_matches).key,
                    list_get_front(transition_state_matches).value);
            list_pop_front(transition_state_matches);
        }
        list_free(transition_state_matches);
        list_pop_front(state_transition_matches);
    }
    list_free(state_transition_matches);
}

_bitset_t* _size_t_char_compute_transition_set(_size_t_char_nfa_t *nfa, Map(size_t, size_t_set_ptr_t) *epsilon_reachable_map,
    _bitset_t *next_set, _char_nfa_transition_t transition) {
    _bitset_t *transition_set = bitset_new();
    list_push_back(nfa->free_state_set_list, transition_set); /* the nfa owns this set! */
    List(size_t) *state_list = set_get_list(next_set);
    while(0 < list_size(state_list)) {
        size_t current_state = list_get_front(state_list);
        if(0 == map_count(nfa->transition_map, current_state)
            || 0 == map_count(map_at(nfa->transition_map, current_state), transition)) {
            list_pop_front(state_list);
            continue;
        }
        _bitset_t* direct_transition_states = map_at(map_at(nfa->transition_map, current_state), transition);
        List(size_t) *dts_list = set_get_list(direct_transition_states);
        while(0 < list_size(dts_list)) {
            set_union(transition_set, map_at(epsilon_reachable_map, list_get_front(dts_list)));
            list_pop_front(dts_list);
        }
        list_free(dts_list);
        list_pop_front(state_list);
    }
    list_free(state_list);
    return transition_set;
}

void _size_t_char_load_dfa_with_transition(Dfa(size_t_set_ptr_t, char) *new_dfa,
    _bitset_t *from, _char_nfa_transition_t transition, _bitset_t *to) {
    if(NONE == transition.transition_type) {
        /* construct_int_char_transition_string(from, transition.val, to); TODO delete this!! */
        dfa_add_transition(new_dfa, from, transition.val, to);
    }
}

void _size_t_char_update_accept_status_in_dfa(Dfa(size_t_set_ptr_t, char) *new_dfa, _size_t_char_nfa_t *nfa,
    _bitset_t *states) {
    List(size_t) *list_of_states = set_get_list(states);
    while(0 < list_size(list_of_states)) {
        if(1 == set_count(nfa->accept_states, list_get_front(list_of_states))) {
            dfa_add_accept_state(new_dfa, states);
            list_free(list_of_states);
            return;
        }
        list_pop_front(list_of_states);
    }
    list_free(list_of_states);
}

void _size_t_char_process_next_state_set(Dfa(size_t_set_ptr_t, char) *new_dfa,
    _size_t_char_nfa_t *nfa, Map(size_t, size_t_set_ptr_t) *epsilon_reachable_map, List(size_t_set_ptr_t)* state_queue,
    _bitset_t* next_set) {
    List(size_t) *state_list = set_get_list(next_set);
    _size_t_char_update_accept_status_in_dfa(new_dfa, nfa, next_set);
    Set(_char_nfa_transition_t) *seen_transitions = set_new(_char_nfa_transition_t);
    while(0 < list_size(state_list)) {
        if(0 == map_count(nfa->transition_map, list_get_front(state_list))) {
            list_pop_front(state_list);
            continue;
        }
        List(__char_nfa_transition_t_size_t_set_ptr_t_map_match_t) *transition_list
            = map_get_list(map_at(nfa->transition_map, list_get_front(state_list)));
        while(0 < list_size(transition_list)) {
            if(1 == set_count(seen_transitions, list_get_front(transition_list).key)) {
                list_pop_front(transition_list);
                continue;
            }
            set_insert(seen_transitions, list_get_front(transition_list).key);
            _bitset_t* transition_state = _size_t_char_compute_transition_set(nfa,
                epsilon_reachable_map, next_set, list_get_front(transition_list).key);
            _size_t_char_load_dfa_with_transition(new_dfa, next_set, list_get_front(transition_list).key,
                transition_state);
            _size_t_char_update_accept_status_in_dfa(new_dfa, nfa, transition_state);
            list_push_back(state_queue, transition_state);
            list_pop_front(transition_list);
        }
        list_pop_front(state_list);
        list_free(transition_list);
    }
    set_free(seen_transitions);
    list_free(state_list);
}

void _size_t_char_construct_dfa_with_transitions_and_epsilon_reachable_map(Dfa(size_t_set_ptr_t, char) *new_dfa,
    _size_t_char_nfa_t *nfa, Map(size_t, size_t_set_ptr_t) *epsilon_reachable_map) {
    List(size_t_set_ptr_t) *state_queue = list_new(size_t_set_ptr_t);
    Set(size_t_set_ptr_t) *seen = set_new(size_t_set_ptr_t);
    set_set_hash(seen, &_bitset_collection_hash);
    set_set_value_equals(seen, &_bitset_equals_);
    list_push_back(state_queue, map_at(epsilon_reachable_map, nfa->begin_state));
    while(0 < list_size(state_queue)) {
        _bitset_t *next_set = list_get_front(state_queue);
        if(0 == set_count(seen, next_set)) {
            _size_t_char_process_next_state_set(new_dfa, nfa, epsilon_reachable_map, state_queue, next_set);
            set_insert(seen, next_set);
        }
        list_pop_front(state_queue);
    }
    list_free(state_queue);
    set_free(seen);
}

/* Hash and equals overrides for the transition. */
size_t _size_t_char_transition_hash_fn(_size_t_set_ptr_t_char_transition_t transition) {
    return _bitset_collection_hash(transition.state);
}
size_t _size_t_char_transition_equals_fn(_size_t_set_ptr_t_char_transition_t transition1,
    _size_t_set_ptr_t_char_transition_t transition2) {
    return transition1.transition == transition2.transition
        && _bitset_equals_(transition1.state, transition2.state);
}

void _size_t_char_override_dfa_tranisition_map_equals(Dfa(size_t_set_ptr_t, char) *new_dfa) {\
    map_set_hash(new_dfa->transition_map, &_size_t_char_transition_hash_fn);
    map_set_key_eq(new_dfa->transition_map, &_size_t_char_transition_equals_fn);
    set_set_hash(new_dfa->accept_states, &_bitset_collection_hash);
    set_set_value_equals(new_dfa->accept_states, &_bitset_equals_);
}

/* Converts an NFA into a DFA using the powerset algorithm. This mutates the current NFA, so this algorithm can
    only be called *once*. */
Dfa(size_t_set_ptr_t, char)* _size_t_char_nfa_to_dfa(_size_t_char_nfa_t *nfa, Vector(char) *alphabet_set) {
    Map(size_t, size_t_set_ptr_t) *epsilon_reachable_map = map_new(size_t, size_t_set_ptr_t);
    _size_t_char_fill_epsilon_reachable_map(nfa, epsilon_reachable_map);
    _size_t_char_replace_all_transitions_with_alphabet_set_transitions(nfa, alphabet_set);
    Dfa(size_t_set_ptr_t, char) *new_dfa = dfa_new(size_t_set_ptr_t, char, map_at(epsilon_reachable_map, nfa->begin_state));
    _size_t_char_override_dfa_tranisition_map_equals(new_dfa);
    _size_t_char_construct_dfa_with_transitions_and_epsilon_reachable_map(new_dfa, nfa, epsilon_reachable_map);
    map_free(epsilon_reachable_map);
    return new_dfa;
}

void _size_t_char_nfa_free(_size_t_char_nfa_t *nfa) {
    /* TODO this acharempt at freeing the nfa is INCOMPLETE! */
    /* (1) Free the transition map */
    List(_size_t__size_t_char_nfa_transition_map_t_map_match_t) *state_transition_matches = map_get_list(nfa->transition_map);
    while(0 < list_size(state_transition_matches)) {
        List(__char_nfa_transition_t_size_t_set_ptr_t_map_match_t) *transition_state_matches = map_get_list(map_at(nfa->transition_map,
            list_get_front(state_transition_matches).key));
        while(0 < list_size(transition_state_matches)) {
            set_free(list_get_front(transition_state_matches).value);
            list_pop_front(transition_state_matches);
        }
        list_free(transition_state_matches);
        map_free(map_at(nfa->transition_map, list_get_front(state_transition_matches).key));
        list_pop_front(state_transition_matches);
    }
    list_free(state_transition_matches);
    map_free(nfa->transition_map);
    
    /* (2) Free the accept_states set */
    set_free(nfa->accept_states);
    
    /* (3) Free the all_states set */
    set_free(nfa->all_states);
    
    /* (4) Free the epsilon_map map */
    List(_size_t_size_t_set_ptr_t_map_match_t) *epsilon_transitions = map_get_list(nfa->epsilon_map);
    while(0 < list_size(epsilon_transitions)) {
        set_free(list_get_front(epsilon_transitions).value);
        list_pop_front(epsilon_transitions);
    }
    list_free(epsilon_transitions);
    map_free(nfa->epsilon_map);
    
    while(0 < list_size(nfa->free_state_set_list)) {
        set_free(list_get_front(nfa->free_state_set_list));
        list_pop_front(nfa->free_state_set_list);
    }
    list_free(nfa->free_state_set_list);
    
    /* (5) Free the original nfa */
    free(nfa);
}

struct _size_t_char_nfa_fns_ size_t_char_nfa_fns = {
    &_size_t_char_nfa_add_transition, &_size_t_char_nfa_add_epsilon_transition,
    &_size_t_char_nfa_add_alphabet_transition, &_size_t_char_nfa_remove_transition,
    &_size_t_char_nfa_remove_epsilon_transition, &_size_t_char_nfa_remove_alphabet_transition,
    &_size_t_char_nfa_add_accept_state, &_size_t_char_nfa_remove_accept_state,
    &_size_t_char_nfa_to_dfa, &_size_t_char_nfa_free
};

_size_t_char_nfa_t* _size_t_char_nfa_new(size_t begin_state) {
    _size_t_char_nfa_t* new_nfa = (_size_t_char_nfa_t*) malloc(sizeof(_size_t_char_nfa_t));
    new_nfa->transition_map = map_new(size_t, _size_t_char_nfa_transition_map_t);
    new_nfa->begin_state = begin_state;
    new_nfa->all_states = bitset_new();
    new_nfa->accept_states = bitset_new();
    new_nfa->epsilon_map = map_new(size_t, size_t_set_ptr_t);
    new_nfa->free_state_set_list = list_new(size_t_set_ptr_t);
    map_insert(new_nfa->epsilon_map, begin_state, bitset_new());
    new_nfa->fns = &size_t_char_nfa_fns;
    return new_nfa;
}


#endif