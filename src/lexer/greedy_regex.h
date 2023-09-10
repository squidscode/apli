#ifndef REGEX_H
#define REGEX_H

#include <string.h>
#include "dfa.h"
#include <stdio.h>
#include <assert.h>
#ifdef UNOPTIMIZED
#include "nfa.h"
#else
#include "nfa_optimized.h"
#endif

#include "../util/bitset.h"

/**
 * Loads, compiles, and runs a POSIX (ERE) style regex.
 * 
 * ----- Usage -----
 *   Regex *reg = regex_from("...");
 *     - regex_compile(reg)                   -> Regex*
 *     - regex_run(reg, str: const char*)     -> size_t ( 0 or 1 )
 *     - regex_find_all(reg, const char*)     -> List(_regex_match_t)*
 *     - regex_free(reg)                      -> void
 */

#define Regex                       _regex_t
#define regex_from(str)             (_regex_fn_impl_.from((str)))
#define regex_compile(regex)        (_regex_fn_impl_.compile((regex)))
#define regex_run(regex, str)       (_regex_fn_impl_.run((regex), (str)))
#define regex_find_all(regex,str)   (_regex_fn_impl_.find_all_matches((regex), (str)))
#define regex_free(regex)           (_regex_fn_impl_.destroy((regex)))

/**
 * Regex parsing algorithm:
 *   - (1) Split the regex into different capturing groups
 *   ---> a capturing group is a (const char*, size_t) representing
 *        a string segment. Any alternating symbols in the base (ie.)
 *        not nested in any '(' or ')' characters are considered splits
 *        between different capturing groups.
 *   - (2) If len(capturing_groups) == 1:
 *      - EXPAND!
 *   - (3) else:
 *      - for(capture_group : capturing_groups):
 *            add_epsilon(start, next_free_state)
 *            end_states.append(PARSE(capture_group, next_free_state))
 *            next_free_state = end_states[-1] + 1
 *        for each end_state in end_states:
 *            add_epsilon(end_state, next_free_state)
 *        return next_free_state
 */

#define init_regex() \
    define_list(char); \
    define_list(int); \
    init_dfa_types(size_t, char); \
    typedef struct _size_t_char_dfa_ _size_t_char_dfa_t; \
    _size_t_char_dfa_t* _size_t_char_dfa_new(size_t); \
    init_nfa(size_t, char); \
    define_dfa(size_t, char); \
    define_nfa(size_t, char)

struct _regex_;
struct _regex_match_;

struct _regex_match_ {
    size_t begin;
    size_t length;
};
typedef struct _regex_match_ _regex_match_t;


typedef struct _regex_ _regex_t;

typedef struct __regex_match_t_list_ _regex_match_t_list_t;
struct _regex_fns_ {
    struct _regex_* (*from)(const char*);
    struct _regex_* (*compile)(struct _regex_*);
    size_t (*run)(struct _regex_*, const char*);
    List(_regex_match_t)* (*find_all_matches)(struct _regex_*, const char*);
    void (*destroy)(struct _regex_*);
};
typedef struct _regex_fns_ _regex_fns_t;

/* The virtual table for the regex functions are exposed, if the user wants to 
   override one of the regex functions. */
extern _regex_fns_t _regex_fn_impl_;

define_list(_regex_match_t);

#define IS_SPECIAL_CHARACTER(c) ('*' == c || '+' == c || '?' == c)

typedef enum {REGEX_RAW_LOADED, REGEX_COMPILED} _regex_state_type;
typedef enum {REGEX_NOT_ROOTED, REGEX_LEFT_ROOTED, REGEX_RIGHT_ROOTED} _regex_root_type;
struct _regex_ {
    _regex_state_type state;
    char root_type;
    const char* raw_regex;
    Nfa(size_t, char) *forward_nfa;
    Nfa(size_t, char) *backward_nfa;
    Dfa(size_t, char) *forward_dfa;
    Dfa(size_t, char) *backward_dfa;
};

_regex_t* _regex_from(const char* str) {
    size_t str_size = strlen(str);
    char *buf = (char*) malloc(str_size * sizeof(char) + 1);
    strcpy(buf, str);
    _regex_t *new_regex = (_regex_t*) malloc(sizeof(_regex_t));
    new_regex->state = REGEX_RAW_LOADED;
    new_regex->root_type = ('^' == str[0] ? REGEX_LEFT_ROOTED : REGEX_NOT_ROOTED)
        | ('$' == str[str_size - 1] ? REGEX_RIGHT_ROOTED : REGEX_NOT_ROOTED);
    new_regex->raw_regex = buf;
    new_regex->forward_nfa = NULL;
    new_regex->backward_nfa = NULL;
    new_regex->forward_dfa = NULL;
    new_regex->backward_dfa = NULL;
    return new_regex;
}

/* _regex_string_segment is used for tokenizing in the regex_parse functions. */
typedef struct _regex_string_segment_ {
    const char* ptr;
    size_t length;
} _regex_string_segment_t;
define_list(_regex_string_segment_t);

/* Forward declarations for the _regex_compile function */
typedef enum {REGEX_FORWARD, REGEX_BACKWARD} _regex_parse_direction_t;
size_t _regex_parse(Vector(char)*, Nfa(size_t, char)*, size_t, const char*, size_t, _regex_parse_direction_t);
List(_regex_string_segment_t)* _regex_split_by_capturing_groups(const char* ptr, size_t size);
List(_regex_string_segment_t)* _regex_split_by_tokens(const char* ptr, size_t size, _regex_parse_direction_t);
/* NOTE: The interval is defined as [start, end). The character at ptr[end] is not counted. */
_regex_string_segment_t _regex_string_segment_from(const char *ptr, size_t start, size_t end);
size_t _regex_expand(Vector(char)*, Nfa(size_t, char)*, size_t, const char*, size_t, _regex_parse_direction_t);
size_t _regex_expand_with_root_tokens(Vector(char) *alphabet, Nfa(size_t, char)*, size_t, const char*, size_t, _regex_parse_direction_t);
size_t _regex_expand_token(Vector(char)*, Nfa(size_t, char)*, size_t, const char*, size_t, _regex_parse_direction_t);
size_t _regex_string_segment_equals(const char*, _regex_string_segment_t segment);
size_t _regex_is_special_character(_regex_string_segment_t);
void _regex_add_all_alphabet_transitions_between(Vector(char) *alphabet, Nfa(size_t, char) *nfa, size_t state1, size_t state2);
void _debug_print_regex_string_segment_t(_regex_string_segment_t rss);
void _regex_expand_root_at_start_token_check(Vector(char) *alphabet, Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end);
void _regex_expand_root_at_start_token(Vector(char) *alphabet, Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end);
void _regex_expand_non_root_token(Vector(char)*, Nfa(size_t, char)*, List(_regex_string_segment_t)*, _regex_string_segment_t *, size_t *, size_t *, _regex_parse_direction_t);
void _regex_expand_root_at_end_token(Vector(char) *alphabet, Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end);
void _regex_expand_root_at_end_token_action(Vector(char) *alphabet, Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end);
void _regex_process_repeat_token(size_t *from, size_t *to, const char *raw_regex, size_t raw_regex_size);

// The alphabet set is global, and I know this is bad design, but it's not `extern' so it's fine! (irony)

#include <stdio.h> // TODO: delete this

_regex_t* _regex_compile(_regex_t *regex) {
    size_t regex_size = strlen(regex->raw_regex);
    regex->forward_nfa = nfa_new(size_t, char, 0);
    regex->backward_nfa = nfa_new(size_t, char, 0);
    Vector(char) *alphabet = vector_new(char);
    for(int i = 0; i < 256; ++i) {
        vector_push_back(alphabet, i);
    }
    // printf("forward dfa: \n");
    size_t fend = _regex_parse(alphabet, regex->forward_nfa, 0, regex->raw_regex, regex_size, REGEX_FORWARD);
    // printf("backward dfa: \n");
    size_t bend = _regex_parse(alphabet, regex->backward_nfa, 0, regex->raw_regex, regex_size, REGEX_BACKWARD);
    // printf("[`%s`] # of nfa states: %zu, ", regex->raw_regex, end + 1);
    nfa_add_accept_state(regex->forward_nfa, fend);
    nfa_add_accept_state(regex->backward_nfa, bend);
    Dfa(size_t_set_ptr_t, char) *forward_dfa = nfa_to_dfa(regex->forward_nfa, alphabet);
    Dfa(size_t_set_ptr_t, char) *backward_dfa = nfa_to_dfa(regex->backward_nfa, alphabet);
    // printf("# of dfa transitions: %zu\n", map_size(dfa->transition_map));
    vector_free(alphabet);
    Dfa(size_t, char) *compressed_forward_dfa = dfa_compress(forward_dfa); // added step
    Dfa(size_t, char) *compressed_backward_dfa = dfa_compress(backward_dfa); // added step
    // printf("Begin state: %zu\n", compressed_forward_dfa->begin_state);
    // List(size_t) *lst = set_get_list(compressed_forward_dfa->accept_states);
    // while(list_size(lst)) {
    //     printf("Accept state: %zu\n", list_get_front(lst));
    //     list_pop_front(lst);
    // }

    free(forward_dfa);
    free(backward_dfa);
    regex->forward_dfa = compressed_forward_dfa;
    regex->backward_dfa = compressed_backward_dfa;
    regex->state = REGEX_COMPILED;
    return regex;
}

/* TODO the following:
 * - free the original set nfa.
 */

// IF begin_expansion_state == 0, then _regex_expand_with_root_tokens is called, otherwise, _regex_expand is called.
size_t _regex_parse(Vector(char) *alphabet, Nfa(size_t, char) *nfa, size_t begin_expansion_state,
    const char* raw_regex, size_t raw_regex_size, _regex_parse_direction_t pd) {
    List(_regex_string_segment_t) *capturing_groups = _regex_split_by_capturing_groups(raw_regex, raw_regex_size);
    List(size_t) *capturing_groups_ends = list_new(size_t);
    // printf("capturing groups size : %zu\n", list_size(capturing_groups));
    size_t next_start = begin_expansion_state + 1;
    size_t end = begin_expansion_state - 1;
    // Using a function pointer here means no 'if' statement inside the while loop. This is faster.
    // printf("begin_expansion_state: %zu\n", begin_expansion_state);
    size_t (*fn)(Vector(char)*, _size_t_char_nfa_t *, size_t, const char *, size_t, _regex_parse_direction_t) = (0 == begin_expansion_state) 
        ? &_regex_expand_with_root_tokens 
        : &_regex_expand;
    while(0 < list_size(capturing_groups)) {
        _regex_string_segment_t segment = list_get_front(capturing_groups);
        // _debug_print_regex_string_segment_t(segment);
        nfa_add_epsilon_transition(nfa, begin_expansion_state, next_start);
        list_push_back(capturing_groups_ends,
            (end = (*fn)(alphabet, nfa, next_start, segment.ptr, segment.length, pd)));
        next_start = end + 1;
        list_pop_front(capturing_groups);
    }
    end += 1; /* this value now represents the "end state" of the list of capturing groups. */
    while(0 < list_size(capturing_groups_ends)) {
        nfa_add_epsilon_transition(nfa, list_get_front(capturing_groups_ends), end);
        list_pop_front(capturing_groups_ends);
    }
    list_free(capturing_groups);
    list_free(capturing_groups_ends);
    return end;
}

/* Invariant: 0 < list_size(groups) */
List(_regex_string_segment_t)* _regex_split_by_capturing_groups(const char *ptr, size_t size) {
    List(_regex_string_segment_t) *groups = list_new(_regex_string_segment_t);
    size_t paren_level = 0;
    size_t capture_group_begin_offset = 0;
    for(size_t i = 0; i < size; ++i) {
        if('\\' == ptr[i]) {
            i += 1; // ignore the next character ALWAYS!
            continue;
        }
        paren_level += ('(' == ptr[i]) - (')' == ptr[i]);
        // if('(' == ptr[i])
        //     paren_level += 1;
        // if(')' == ptr[i])
        //     paren_level -= 1;
        if('|' == ptr[i] && 0 == paren_level) {
            list_push_back(groups, _regex_string_segment_from(ptr, capture_group_begin_offset, i));
            capture_group_begin_offset = i + 1;
        }
        if (0 > paren_level)
            assert(0 == "Incorrectly formatted regex: '(' and ')' tokens are badly-formed.");
    }
    if (0 != paren_level)
        assert(0 == "Incorrectly formatted regex: '(' and ')' tokens are badly-formed.");
    if (size != capture_group_begin_offset)
        list_push_back(groups, _regex_string_segment_from(ptr, capture_group_begin_offset, size));
    return groups;
}

_regex_string_segment_t _regex_string_segment_from(const char *ptr, size_t start, size_t end) {
    _regex_string_segment_t segment = {&ptr[start], end - start};
    return segment;
}

size_t _regex_string_segment_equals(const char* str, _regex_string_segment_t segment) {
    if (strlen(str) != segment.length)
        return 0;
    for(int i = 0; i < segment.length; ++i)
        if(str[i] != segment.ptr[i])
            return 0;
    return 1;
}

// WITH ROOT TOKENS (^ and $)
size_t _regex_expand_with_root_tokens(Vector(char) *alphabet, Nfa(size_t, char) *nfa, 
    size_t begin_expansion_state, const char* raw_regex, size_t raw_regex_size, _regex_parse_direction_t pd) {
    /* the raw_regex string must not have multiple segments (ie. there are no level 0 alternation signs). */
    List(_regex_string_segment_t) *tokens = _regex_split_by_tokens(raw_regex, raw_regex_size, pd);
    Iterator(_regex_string_segment_t) *iter = list_get_iterator(tokens);
    // printf("List of tokens:\n");
    // while(iter_is_not_null(iter)) {
    //     printf("- "); _debug_print_regex_string_segment_t(iter_val(iter));
    //     iter = iter_next(iter);
    // }
    size_t next_start = begin_expansion_state;
    size_t end = begin_expansion_state - 1;
    /* The regex token parser uses 1 look-ahead. */
    _regex_string_segment_t current = list_get_front(tokens);
    // printf("[expand]: "); _debug_print_regex_string_segment_t(current);
    list_pop_front(tokens);

    // Assume rooted at start!
    if(_regex_string_segment_equals("^", current))
        (current = list_get_front(tokens), list_pop_front(tokens));
    
    _regex_expand_non_root_token(alphabet, nfa, tokens, &current, &next_start, &end, pd);
    
    // Assume rooted at end!

    return end;
}

// WITHOUT ROOT TOKENS.
size_t _regex_expand(Vector(char) *alphabet, Nfa(size_t, char) *nfa, size_t begin_expansion_state, const char* raw_regex, size_t raw_regex_size, _regex_parse_direction_t pd) {
    /* the raw_regex string must not have multiple segments (ie. there are no level 0 alternation signs). */
    List(_regex_string_segment_t) *tokens = _regex_split_by_tokens(raw_regex, raw_regex_size, pd);
    size_t next_start = begin_expansion_state;
    size_t end = begin_expansion_state - 1;
    /* The regex token parser uses 1 look-ahead. */
    _regex_string_segment_t current = list_get_front(tokens);
    // printf("[expand]: "); _debug_print_regex_string_segment_t(current);
    list_pop_front(tokens);
    _regex_expand_non_root_token(alphabet, nfa, tokens, &current, &next_start, &end, pd);
    return end;
}

void _regex_expand_root_at_start_token(Vector(char) *alphabet, Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end) {
    nfa_add_epsilon_transition(nfa, (*next_start), (*next_start) + 1);
    _regex_add_all_alphabet_transitions_between(alphabet, nfa, (*next_start) + 1, (*next_start) + 1);
    nfa_add_epsilon_transition(nfa, (*next_start) + 1, (*next_start) + 2);
    *next_start = (*next_start) + 2;
    *end = *next_start;
}

void _regex_expand_root_at_start_token_check(Vector(char) *alphabet, Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end) {
    if (!_regex_string_segment_equals("^", *current)) { // if NOT rooted to the beginning
        _regex_expand_root_at_start_token(alphabet, nfa, current, next_start, end);
    }
}

void _regex_expand_non_root_token(Vector(char)* alphabet, Nfa(size_t, char) *nfa, List(_regex_string_segment_t) *tokens,
    _regex_string_segment_t *current, size_t *next_start, size_t *end, _regex_parse_direction_t pd) {
    while (0 < list_size(tokens)) {
        _regex_string_segment_t next = list_get_front(tokens);
        // printf("[expand]: "); _debug_print_regex_string_segment_t(next);
        if(_regex_is_special_character(*current)) {
            // do nothing
        } else if (_regex_string_segment_equals("*", next)) {
            nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
            *end = _regex_expand_token(alphabet, nfa, (*next_start)+1, current->ptr, current->length, pd);
            nfa_add_epsilon_transition(nfa, (*end), (*next_start) + 1);
            nfa_add_epsilon_transition(nfa, (*end), (*end)+1);
            nfa_add_epsilon_transition(nfa, (*next_start), (*end)+1);
            *end = (*end) + 1;
        } else if (_regex_string_segment_equals("?", next)) {
            nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
            *end = _regex_expand_token(alphabet, nfa, (*next_start)+1, current->ptr, current->length, pd);
            nfa_add_epsilon_transition(nfa, (*end), (*end)+1);
            nfa_add_epsilon_transition(nfa, (*next_start), (*end)+1);
            *end = (*end) + 1;
        } else if (_regex_string_segment_equals("+", next)) {
            nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
            *end = _regex_expand_token(alphabet, nfa, (*next_start)+1, current->ptr, current->length, pd);
            nfa_add_epsilon_transition(nfa, (*end), (*end)+1);
            nfa_add_epsilon_transition(nfa, (*end), (*next_start) + 1);
            *end = (*end) + 1;
        } else if (0 < next.length && '{' == next.ptr[0] && '}' == next.ptr[next.length - 1]) {
            size_t from = 0; size_t to = 0; from -= 1; to -= 1; // setup
            _regex_process_repeat_token(&from, &to, next.ptr, next.length);
            // printf("from: %zu, to: %zu\n", from, to);
            nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
            *next_start = (*next_start) + 1;
            for(size_t i = 0; i < from; ++i) {
                *end = _regex_expand_token(alphabet, nfa, *next_start, current->ptr, current->length, pd);
                *next_start = *end;
            }
            if (0 != to + 1) {
                nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
                *next_start = (*next_start) + 1;
                List(size_t) *list_of_ends = list_new(size_t);
                list_push_front(list_of_ends, *next_start);
                for(size_t i = from; i < to; ++i) {
                    *end = _regex_expand_token(alphabet, nfa, *next_start, current->ptr, current->length, pd);
                    list_push_back(list_of_ends, *end);
                    *next_start = *end;
                }
                *end = *end + 1;
                while(list_size(list_of_ends)) {
                    nfa_add_epsilon_transition(nfa, list_get_front(list_of_ends), *end);
                    list_pop_front(list_of_ends);
                }
            } else { // the '*' expansion:
                nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
                *end = _regex_expand_token(alphabet, nfa, (*next_start)+1, current->ptr, current->length, pd);
                nfa_add_epsilon_transition(nfa, (*end), (*next_start) + 1);
                nfa_add_epsilon_transition(nfa, (*end), (*end)+1);
                nfa_add_epsilon_transition(nfa, (*next_start), (*end)+1);
                *end = (*end) + 1;
            }
        } else {
            (*end) = _regex_expand_token(alphabet, nfa, (*next_start), current->ptr, current->length, pd);
        }
        *next_start = (*end);
        *current = next;
        list_pop_front(tokens);
    }
    if (!_regex_is_special_character(*current) 
     && !_regex_string_segment_equals("$", *current)) { // NOT a special character and not '$'
        (*end) = _regex_expand_token(alphabet, nfa, (*next_start), current->ptr, current->length, pd);
        *next_start = (*end);
    }
}

void _regex_process_repeat_token(size_t *from, size_t *to, const char *raw_regex, size_t raw_regex_size) {
    char buf[10]; size_t ind = 1;
    while(ind < raw_regex_size - 1 && ',' != raw_regex[ind]) 
        (buf[ind - 1] = raw_regex[ind], ind += 1);
    buf[ind - 1] = '\0';
    assert(0 != ind - 1); // needs to have a size greater than 0.
    assert(0 != (*from = atoi(buf)));
    if (ind == raw_regex_size - 1 || '}' == raw_regex[ind]) {
        *to = *from;
        return;
    }
    ind += 1; size_t offset = ind;
    while(ind < raw_regex_size - 1 && '}' != raw_regex[ind]) 
        (buf[ind - offset] = raw_regex[ind], ind += 1);
    buf[ind - offset] = '\0';
    if(0 != ind - offset)
        assert(0 != (*to = atoi(buf)));
}

void _regex_expand_root_at_end_token_action(Vector(char) *alphabet, Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end) {
    nfa_add_epsilon_transition(nfa, *next_start, *next_start + 1);
    _regex_add_all_alphabet_transitions_between(alphabet, nfa, *next_start + 1, *next_start + 1);
    nfa_add_epsilon_transition(nfa, *next_start + 1, *next_start + 2);
    *next_start = *next_start + 2;
    *end = *next_start;
}

void _regex_expand_root_at_end_token(Vector(char) *alphabet, Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end) {
    if (!_regex_string_segment_equals("$", *current)) { // if NOT rooted to the end
        _regex_expand_root_at_end_token_action(alphabet, nfa, current, next_start, end);
    }
}


size_t _regex_is_special_character(_regex_string_segment_t rss) {
    return _regex_string_segment_equals("*", rss) || _regex_string_segment_equals("?", rss)
            || _regex_string_segment_equals("+", rss)
            || (0 < rss.length && '{' == rss.ptr[0] && '}' == rss.ptr[rss.length - 1]);
}

size_t _regex_expand_token(Vector(char) *alphabet, Nfa(size_t, char) *nfa, size_t begin_expansion_state, const char* raw_regex, size_t raw_regex_size, _regex_parse_direction_t pd) {
    if(0 >= raw_regex_size)
        assert(0 == "Invalid regex_segment size.");
    if (1 == raw_regex_size) {
        nfa_add_transition(nfa, begin_expansion_state, raw_regex[0], begin_expansion_state + 1);
        return begin_expansion_state + 1;
    } else if (2 == raw_regex_size && '\\' == raw_regex[0]) {
        if('n' == raw_regex[1])
            nfa_add_transition(nfa, begin_expansion_state, '\n', begin_expansion_state + 1);
        else if('r' == raw_regex[1])
            nfa_add_transition(nfa, begin_expansion_state, '\r', begin_expansion_state + 1);
        else if('t' == raw_regex[1])
            nfa_add_transition(nfa, begin_expansion_state, '\t', begin_expansion_state + 1);
        else
            nfa_add_transition(nfa, begin_expansion_state, raw_regex[1], begin_expansion_state + 1);
        return begin_expansion_state + 1;
    } else if ('(' == raw_regex[0] && ')' == raw_regex[raw_regex_size - 1]) {
        return _regex_parse(alphabet, nfa, begin_expansion_state, &raw_regex[1], raw_regex_size - 2, pd);
    } else if ('[' == raw_regex[0] && ']' == raw_regex[raw_regex_size - 1]) {
        size_t start_index = 1;
        char inverted_flag = 0;
        if('^' == raw_regex[1]) {
            start_index = 2;
            inverted_flag = 1;
            _regex_add_all_alphabet_transitions_between(alphabet, nfa, begin_expansion_state, begin_expansion_state + 1);
        }
        size_t end_index = raw_regex_size - 1;
        for(size_t ind = start_index; ind < end_index; ++ind) {
            if(ind + 2 < end_index && '\\' != raw_regex[ind] && '-' == raw_regex[ind+1]) {
                // Indicates a character span
                char start = raw_regex[ind];
                char end = raw_regex[ind+2];
                if(end < start)
                    assert(0 == "Invalid character span (end_char < start_char)");
                for(char c = start; c <= end; ++c)
                    if(inverted_flag)
                        nfa_remove_transition(nfa, begin_expansion_state, c, begin_expansion_state + 1);
                    else
                        nfa_add_transition(nfa, begin_expansion_state, c, begin_expansion_state + 1);
                ind += 2; // we have processed raw_regex[ind + 2]
            } else {
                // No character span
                if(inverted_flag)
                    nfa_remove_transition(nfa, begin_expansion_state, raw_regex[ind], begin_expansion_state + 1);
                else
                    nfa_add_transition(nfa, begin_expansion_state, raw_regex[ind], begin_expansion_state + 1);
            }
        }
        return begin_expansion_state + 1;
    }
    _regex_string_segment_t rss = {raw_regex, raw_regex_size};
    // _debug_print_regex_string_segment_t(rss);
    assert(0 == "Invalid regex format.");
}

void _regex_add_all_alphabet_transitions_between(Vector(char) *alphabet, Nfa(size_t, char) *nfa, size_t state1, size_t state2) {
    size_t alphabet_sz = vector_size(alphabet);
    for(size_t i = 0; i < alphabet_sz; ++i) {
        nfa_add_transition(nfa, state1, vector_get(alphabet, i), state2);
    }
}

#define _regex_list_append_on_parse_direction(list, elm) \
    { \
        if(pd == REGEX_FORWARD) { \
            list_push_back((list), (elm)); \
        } else { \
            list_push_front((list), (elm)); \
        } \
    }

/* Invariant: the size of the list that is returned is at least 1. */
List(_regex_string_segment_t)* _regex_split_by_tokens(const char *ptr, size_t size, _regex_parse_direction_t pd) {
    List(_regex_string_segment_t) *tokens = list_new(_regex_string_segment_t);
    size_t paren_level = 0; size_t bracket_level = 0; size_t curly_bracket_level = 0;
    size_t capture_group_begin_offset = 0;
    for(size_t i = 0; i < size; ++i) {
        if ('\\' == ptr[i] && 0 == paren_level && 0 == bracket_level) {
            i += 1;
            _regex_list_append_on_parse_direction(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
            capture_group_begin_offset = i + 1;
        } else if ('(' != ptr[i] && ')' != ptr[i] && 0 == paren_level
            && '[' != ptr[i] && ']' != ptr[i] && 0 == bracket_level
            && '{' != ptr[i] && '}' != ptr[i] && 0 == curly_bracket_level) {
            _regex_list_append_on_parse_direction(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
            if(pd == REGEX_BACKWARD && IS_SPECIAL_CHARACTER(ptr[i])) { // condition to re-order token.
                // printf("REORDERING!\n");
                _regex_string_segment_t p1 = list_get_front(tokens); 
                // printf("p1: "); _debug_print_regex_string_segment_t(p1);
                list_pop_front(tokens);
                _regex_string_segment_t p2 = list_get_front(tokens);
                // printf("p2: "); _debug_print_regex_string_segment_t(p2); 
                list_pop_front(tokens);
                list_push_front(tokens, p1);
                list_push_front(tokens, p2);
            }
            capture_group_begin_offset = i + 1;
        } else if ('[' == ptr[i] && 0 == paren_level) {
            if (0 == bracket_level) {
                capture_group_begin_offset = i;
                bracket_level += 1;
            }
        } else if ('(' == ptr[i]) {
            if (0 == paren_level)
                capture_group_begin_offset = i;
            paren_level += 1;
        } else if (']' == ptr[i] && 1 == bracket_level && '[' != ptr[i-1] \
            && 0 == paren_level) {
            bracket_level -= 1;
            _regex_list_append_on_parse_direction(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
            capture_group_begin_offset = i + 1;
        } else if (')' == ptr[i] && 1 == paren_level) {
            paren_level -= 1;
            _regex_list_append_on_parse_direction(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
            capture_group_begin_offset = i + 1;
        } else if (']' == ptr[i] && 0 == paren_level) {
            if ('[' != ptr[i-1])
                bracket_level -= 1;
        } else if (')' == ptr[i]) {
            paren_level -= 1;
        } else if ('{' == ptr[i] && 0 == paren_level) {
            curly_bracket_level += 1;
            capture_group_begin_offset = i;
        } else if ('}' == ptr[i] && 0 == paren_level) {
            curly_bracket_level -= 1;
            _regex_list_append_on_parse_direction(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
            if(pd == REGEX_BACKWARD) {
                // printf("REORDERING!\n");
                _regex_string_segment_t p1 = list_get_front(tokens); 
                // printf("p1: "); _debug_print_regex_string_segment_t(p1);
                list_pop_front(tokens);
                _regex_string_segment_t p2 = list_get_front(tokens);
                // printf("p2: "); _debug_print_regex_string_segment_t(p2); 
                list_pop_front(tokens);
                list_push_front(tokens, p1);
                list_push_front(tokens, p2);
            }
            capture_group_begin_offset = i + 1;
        }
    }

    // Fixing displaced '^' and '$' tokens
    _regex_string_segment_t front = list_get_front(tokens);
    _regex_string_segment_t back = list_get_back(tokens);
    int case_1 = front.ptr[0] == '$' && front.length == 1;
    int case_2 = back.ptr[0] == '^' && back.length == 1;
    if (pd == REGEX_BACKWARD && (case_1 || case_2)) {
        if (case_1 && case_2) {
            list_pop_front(tokens);
            list_pop_back(tokens);
            list_push_front(tokens, back);
            list_push_back(tokens, front);
        } else if (case_1) {
            list_pop_front(tokens);
            list_push_back(tokens, front);
        } else if (case_2) {
            list_pop_back(tokens);
            list_push_front(tokens, back);
        }
    }
    if (0 != paren_level || 0 != bracket_level)
        assert(0 == "Incorrectly formatted regex.");
    if (size != capture_group_begin_offset)
        _regex_list_append_on_parse_direction(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, size));
    return tokens;
}

size_t _regex_run(_regex_t *regex, const char *str) {
    if(REGEX_COMPILED != regex->state)
        assert(0 == "A regex cannot be run without first being compiled.");
    size_t str_sz = strlen(str);

    char *rev_str = (char*) malloc(sizeof(char) * (str_sz + 1));
    for(size_t i = 0; i < str_sz; ++i)
        rev_str[str_sz - i - 1] = str[i];
    rev_str[str_sz] = '\0';
    
    size_t right_bound = dfa_run_greedy(
        regex->forward_dfa, 
        str, 
        str_sz
    );
    if(right_bound == ~0UL)
        return 0UL;
    size_t rev_right_bound = dfa_run_greedy(
        regex->backward_dfa, 
        rev_str + (str_sz - right_bound),
        right_bound
    );
    assert(rev_right_bound != ~0UL);
    size_t left_bound = right_bound - rev_right_bound;

    // printf("[%zu, %zu) rrb=%zu\n", left_bound, right_bound, rev_right_bound);

    size_t accept_flag = 1UL;

    accept_flag &= (!(REGEX_LEFT_ROOTED & regex->root_type) || left_bound == 0);
    accept_flag &= (!(REGEX_RIGHT_ROOTED & regex->root_type) || right_bound == str_sz);

    free(rev_str);

    return accept_flag;
}

List(_regex_match_t)* _regex_find_all_regex_matches(_regex_t *regex, const char *str) {
    if(REGEX_COMPILED != regex->state)
        assert(0 == "A regex cannot be run without first being compiled.");
    
    List(_regex_match_t) *matches = list_new(_regex_match_t);
    size_t str_sz = strlen(str);

    char *rev_str = (char*) malloc(sizeof(char) * (str_sz + 1));
    for(size_t i = 0; i < str_sz; ++i)
        rev_str[str_sz - i - 1] = str[i];
    rev_str[str_sz] = '\0';

    _regex_match_t match;
    size_t right_bound, offset = 0, left_bound, rev_right_bound;
    do {
        if(offset >= str_sz) break;
        right_bound = dfa_run_greedy(
            regex->forward_dfa, 
            str + offset, 
            str_sz - offset
        );
        if(right_bound == ~0UL) break;
        right_bound += offset;
        rev_right_bound = dfa_run_greedy(
            regex->backward_dfa, 
            rev_str + (str_sz - right_bound),
            right_bound
        );
        assert(rev_right_bound != ~0UL);
        left_bound = right_bound - rev_right_bound;
        // printf("[%zu, %zu) rrb=%zu\n", left_bound, right_bound, rev_right_bound);
        match.begin = left_bound;
        match.length = right_bound - left_bound;
        list_push_back(matches, match);
        offset = right_bound;
    } while(1);

    free(rev_str);

    return matches;
}

void _regex_destroy(_regex_t *regex) {
    free((void*) regex->raw_regex);
    if(regex->state == REGEX_RAW_LOADED) {
        free(regex);
        return;
    }
    nfa_free(regex->forward_nfa);
    nfa_free(regex->backward_nfa);
    dfa_free(regex->forward_dfa);
    dfa_free(regex->backward_dfa);
    free(regex);
}

_regex_fns_t _regex_fn_impl_ = {
    &_regex_from,
    &_regex_compile,
    &_regex_run,
    &_regex_find_all_regex_matches,
    &_regex_destroy
};

void _debug_print_regex_string_segment_t(_regex_string_segment_t rss) {
    assert(0 == "I am not called!");
    char *buf = (char*) (malloc(rss.length + 1));
    for(size_t i = 0; i < rss.length; ++i)
        buf[i] = rss.ptr[i];
    buf[rss.length] = '\0';
    printf("String Segment: \"%s\"\n", buf);
    free(buf);
}

#endif