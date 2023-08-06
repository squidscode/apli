#include "dfa.h"
#include "nfa.h"
#include <stdio.h>
// #include "regex.h"

init_regex();
define_list(_regex_match_t);

// Have the chunk size start at 100, but eventually narrow down using the MLE of the exponential pdf function:
// MLE = (n - 2) / SUM(X)
#define SEARCH_CHUNK_SIZE   (1000)
#define TOKEN_CHUNK_SIZE    (15)
#define max(x,y)    (((x) < (y)) ? (y) : (x))

typedef enum {REGEX_RAW_LOADED, REGEX_COMPILED} _regex_state_type;
struct _regex_ {
    _regex_state_type state;
    const char* raw_regex;
    Nfa(size_t, char) *nfa;
    Fdfa(size_t_set_ptr_t, char) *fdfa;
};

_regex_t* _regex_from(const char* str) {
    size_t str_size = strlen(str);
    char *buf = (char*) malloc(str_size * sizeof(char) + 1);
    strcpy(buf, str);
    _regex_t *new_regex = (_regex_t*) malloc(sizeof(_regex_t));
    new_regex->state = REGEX_RAW_LOADED;
    new_regex->raw_regex = buf;
    new_regex->nfa = NULL;
    new_regex->fdfa = NULL;
    return new_regex;
}

/* _regex_string_segment is used for tokenizing in the regex_parse functions. */
typedef struct _regex_string_segment_ {
    const char* ptr;
    size_t length;
} _regex_string_segment_t;
define_list(_regex_string_segment_t);

/* Forward declarations for the _regex_compile function */
size_t _regex_parse(Nfa(size_t, char)*, size_t, const char*, size_t);
List(_regex_string_segment_t)* _regex_split_by_capturing_groups(const char* ptr, size_t size);
List(_regex_string_segment_t)* _regex_split_by_tokens(const char* ptr, size_t size);
/* NOTE: The interval is defined as [start, end). The character at ptr[end] is not counted. */
_regex_string_segment_t _regex_string_segment_from(const char *ptr, size_t start, size_t end);
size_t _regex_expand(Nfa(size_t, char)*, size_t, const char*, size_t);
size_t _regex_expand_with_root_tokens(Nfa(size_t, char)*, size_t, const char*, size_t);
size_t _regex_expand_token(Nfa(size_t, char)*, size_t, const char*, size_t);
size_t _regex_string_segment_equals(const char*, _regex_string_segment_t segment);
size_t _regex_is_special_character(_regex_string_segment_t);
void _regex_add_all_alphabet_transitions_between(Nfa(size_t, char) *nfa, size_t state1, size_t state2);
void _debug_print_regex_string_segment_t(_regex_string_segment_t rss);
void _regex_expand_root_at_start_token(Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end);
void _regex_expand_non_root_token(Nfa(size_t, char) *nfa, List(_regex_string_segment_t) *tokens,
    _regex_string_segment_t *current, size_t *next_start, size_t *end);
void _regex_expand_root_at_end_token(Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end);
void _regex_process_repeat_token(size_t *from, size_t *to, const char *raw_regex, size_t raw_regex_size);

// The alphabet set is global, and I know this is bad design, but it's not `extern' so it's fine! (irony)
// TODO pass the alphabet set into the functions
Set(char) *alphabet;

_regex_t* _regex_compile(_regex_t *regex) {
    size_t regex_size = strlen(regex->raw_regex);
    regex->nfa = nfa_new(size_t, char, 0);
    alphabet = set_new(char);
    for(int i = 0; i < 256; ++i) {
        set_insert(alphabet, i);
    }
    size_t end = _regex_parse(regex->nfa, 0, regex->raw_regex, regex_size);
    // printf("start : %zu\nend : %zu\n", 0UL, end);
    nfa_add_accept_state(regex->nfa, end);
    Dfa(size_t_set_ptr_t, char) *dfa = nfa_to_dfa(regex->nfa, alphabet);
    // printf("# of dfa transitions: %zu\n", map_size(dfa->transition_map));
    set_free(alphabet);
    regex->fdfa = dfa_to_fdfa(dfa);
    regex->state = REGEX_COMPILED;
    return regex;
}

/* TODO the following:
 * - compress the final dfa created AND free the original set nfa.
 */

// IF begin_expansion_state == 0, then _regex_expand_with_root_tokens is called, otherwise, _regex_expand is called.
size_t _regex_parse(Nfa(size_t, char) *nfa, size_t begin_expansion_state,
    const char* raw_regex, size_t raw_regex_size) {
    List(_regex_string_segment_t) *capturing_groups = _regex_split_by_capturing_groups(raw_regex, raw_regex_size);
    List(size_t) *capturing_groups_ends = list_new(size_t);
    // printf("capturing groups size : %zu\n", list_size(capturing_groups));
    size_t next_start = begin_expansion_state + 1;
    size_t end = begin_expansion_state - 1;
    // Using a function pointer here means no 'if' statement inside the while loop. This is faster.
    // printf("begin_expansion_state: %zu\n", begin_expansion_state);
    __auto_type fn = (0 == begin_expansion_state) ? &_regex_expand_with_root_tokens : &_regex_expand;
    while(0 < list_size(capturing_groups)) {
        _regex_string_segment_t segment = list_get_front(capturing_groups);
        // _debug_print_regex_string_segment_t(segment);
        nfa_add_epsilon_transition(nfa, begin_expansion_state, next_start);
        list_push_back(capturing_groups_ends,
            (end = (*fn)(nfa, next_start, segment.ptr, segment.length)));
        next_start = end + 1;
        list_pop_front(capturing_groups);
    }
    end += 1; /* this value now reprents the "end state" of the list of capturing groups. */
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
        if('(' == ptr[i])
            paren_level += 1;
        if(')' == ptr[i])
            paren_level -= 1;
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
size_t _regex_expand_with_root_tokens(Nfa(size_t, char) *nfa, size_t begin_expansion_state, const char* raw_regex, size_t raw_regex_size) {
    /* the raw_regex string must not have multiple segments (ie. there are no level 0 alternation signs). */
    List(_regex_string_segment_t) *tokens = _regex_split_by_tokens(raw_regex, raw_regex_size);
    size_t next_start = begin_expansion_state;
    size_t end = begin_expansion_state - 1;
    /* The regex token parser uses 1 look-ahead. */
    _regex_string_segment_t current = list_get_front(tokens);
    // printf("[expand]: "); _debug_print_regex_string_segment_t(current);
    list_pop_front(tokens);
    _regex_expand_root_at_start_token(nfa, &current, &next_start, &end);
    if(_regex_string_segment_equals("^", current))
        (current = list_get_front(tokens), list_pop_front(tokens));
    _regex_expand_non_root_token(nfa, tokens, &current, &next_start, &end);
    _regex_expand_root_at_end_token(nfa, &current, &next_start, &end);
    return end;
}

// WITHOUT ROOT TOKENS.
size_t _regex_expand(Nfa(size_t, char) *nfa, size_t begin_expansion_state, const char* raw_regex, size_t raw_regex_size) {
    /* the raw_regex string must not have multiple segments (ie. there are no level 0 alternation signs). */
    List(_regex_string_segment_t) *tokens = _regex_split_by_tokens(raw_regex, raw_regex_size);
    size_t next_start = begin_expansion_state;
    size_t end = begin_expansion_state - 1;
    /* The regex token parser uses 1 look-ahead. */
    _regex_string_segment_t current = list_get_front(tokens);
    // printf("[expand]: "); _debug_print_regex_string_segment_t(current);
    list_pop_front(tokens);
    _regex_expand_non_root_token(nfa, tokens, &current, &next_start, &end);
    return end;
}

void _regex_expand_root_at_start_token(Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end) {
    if (!_regex_string_segment_equals("^", *current)) { // if NOT rooted to the beginning
        nfa_add_epsilon_transition(nfa, (*next_start), (*next_start) + 1);
        _regex_add_all_alphabet_transitions_between(nfa, (*next_start) + 1, (*next_start) + 1);
        nfa_add_epsilon_transition(nfa, (*next_start) + 1, (*next_start) + 2);
        *next_start = (*next_start) + 2;
        *end = *next_start;
    }
}

void _regex_expand_non_root_token(Nfa(size_t, char) *nfa, List(_regex_string_segment_t) *tokens,
    _regex_string_segment_t *current, size_t *next_start, size_t *end) {
    while (0 < list_size(tokens)) {
        _regex_string_segment_t next = list_get_front(tokens);
        // printf("[expand]: "); _debug_print_regex_string_segment_t(next);
        if(_regex_is_special_character(*current)) {
            // do nothing
        } else if (_regex_string_segment_equals("*", next)) {
            nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
            *end = _regex_expand_token(nfa, (*next_start)+1, current->ptr, current->length);
            nfa_add_epsilon_transition(nfa, (*end), (*next_start) + 1);
            nfa_add_epsilon_transition(nfa, (*end), (*end)+1);
            nfa_add_epsilon_transition(nfa, (*next_start), (*end)+1);
            *end = (*end) + 1;
        } else if (_regex_string_segment_equals("?", next)) {
            nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
            *end = _regex_expand_token(nfa, (*next_start)+1, current->ptr, current->length);
            nfa_add_epsilon_transition(nfa, (*end), (*end)+1);
            nfa_add_epsilon_transition(nfa, (*next_start), (*end)+1);
            *end = (*end) + 1;
        } else if (_regex_string_segment_equals("+", next)) {
            nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
            *end = _regex_expand_token(nfa, (*next_start)+1, current->ptr, current->length);
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
                *end = _regex_expand_token(nfa, *next_start, current->ptr, current->length);
                *next_start = *end;
            }
            if (0 != to + 1) {
                nfa_add_epsilon_transition(nfa, (*next_start), (*next_start)+1);
                *next_start = (*next_start) + 1;
                List(size_t) *list_of_ends = list_new(size_t);
                list_push_front(list_of_ends, *next_start);
                for(size_t i = from; i < to; ++i) {
                    *end = _regex_expand_token(nfa, *next_start, current->ptr, current->length);
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
                *end = _regex_expand_token(nfa, (*next_start)+1, current->ptr, current->length);
                nfa_add_epsilon_transition(nfa, (*end), (*next_start) + 1);
                nfa_add_epsilon_transition(nfa, (*end), (*end)+1);
                nfa_add_epsilon_transition(nfa, (*next_start), (*end)+1);
                *end = (*end) + 1;
            }
        } else {
            (*end) = _regex_expand_token(nfa, (*next_start), current->ptr, current->length);
        }
        *next_start = (*end);
        *current = next;
        list_pop_front(tokens);
    }
    if (!_regex_is_special_character(*current) 
     && !_regex_string_segment_equals("$", *current)) { // NOT a special character and not '$'
        (*end) = _regex_expand_token(nfa, (*next_start), current->ptr, current->length);
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

void _regex_expand_root_at_end_token(Nfa(size_t, char) *nfa, _regex_string_segment_t *current, size_t *next_start, size_t *end) {
    if (!_regex_string_segment_equals("$", *current)) { // if NOT rooted to the end
        nfa_add_epsilon_transition(nfa, *next_start, *next_start + 1);
        _regex_add_all_alphabet_transitions_between(nfa, *next_start + 1, *next_start + 1);
        nfa_add_epsilon_transition(nfa, *next_start + 1, *next_start + 2);
        *next_start = *next_start + 2;
        *end = *next_start;
    }
}


size_t _regex_is_special_character(_regex_string_segment_t rss) {
    return _regex_string_segment_equals("*", rss) || _regex_string_segment_equals("?", rss)
            || _regex_string_segment_equals("+", rss)
            || (0 < rss.length && '{' == rss.ptr[0] && '}' == rss.ptr[rss.length - 1]);
}

size_t _regex_expand_token(Nfa(size_t, char) *nfa, size_t begin_expansion_state, const char* raw_regex, size_t raw_regex_size) {
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
        return _regex_parse(nfa, begin_expansion_state, &raw_regex[1], raw_regex_size - 2);
    } else if ('[' == raw_regex[0] && ']' == raw_regex[raw_regex_size - 1]) {
        size_t start_index = 1;
        char inverted_flag = 0;
        if('^' == raw_regex[1]) {
            start_index = 2;
            inverted_flag = 1;
            _regex_add_all_alphabet_transitions_between(nfa, begin_expansion_state, begin_expansion_state + 1);
        }
        for(size_t ind = start_index; ind < raw_regex_size - 1; ++ind) {
            inverted_flag ? nfa_remove_transition(nfa, begin_expansion_state, raw_regex[ind], begin_expansion_state + 1) : 
                            nfa_add_transition(nfa, begin_expansion_state, raw_regex[ind], begin_expansion_state + 1);
        }
        return begin_expansion_state + 1;
    }
    _regex_string_segment_t rss = {raw_regex, raw_regex_size};
    // _debug_print_regex_string_segment_t(rss);
    assert(0 == "Invalid regex format.");
}

void _regex_add_all_alphabet_transitions_between(Nfa(size_t, char) *nfa, size_t state1, size_t state2) {
    List(char) *alphabet_chars = set_get_list(alphabet);
    while(0 < list_size(alphabet_chars)) {
        nfa_add_transition(nfa, state1, list_get_front(alphabet_chars), state2);
        list_pop_front(alphabet_chars);
    }
    list_free(alphabet_chars);
}

/* Invariant: the size of the list that is returned is at least 1. */
List(_regex_string_segment_t)* _regex_split_by_tokens(const char *ptr, size_t size) {
    List(_regex_string_segment_t) *tokens = list_new(_regex_string_segment_t);
    size_t paren_level = 0; size_t bracket_level = 0; size_t curly_bracket_level = 0;
    size_t capture_group_begin_offset = 0;
    for(size_t i = 0; i < size; ++i) {
        if ('\\' == ptr[i] && 0 == paren_level && 0 == bracket_level) {
            i += 1;
            list_push_back(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
            capture_group_begin_offset = i + 1;
        } else if ('(' != ptr[i] && ')' != ptr[i] && 0 == paren_level
            && '[' != ptr[i] && ']' != ptr[i] && 0 == bracket_level
            && '{' != ptr[i] && '}' != ptr[i] && 0 == curly_bracket_level) {
            list_push_back(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
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
            list_push_back(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
            capture_group_begin_offset = i + 1;
        } else if (')' == ptr[i] && 1 == paren_level) {
            paren_level -= 1;
            list_push_back(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
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
            list_push_back(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
            capture_group_begin_offset = i + 1;
        }
        // if (1 < bracket_level || 0 > bracket_level
        //     || 0 > paren_level
        //     || 1 < curly_bracket_level || 0 > curly_bracket_level)
        //     assert(0 == "Incorrectly formatted regex.");
    }
    if (0 != paren_level || 0 != bracket_level)
        assert(0 == "Incorrectly formatted regex.");
    if (size != capture_group_begin_offset)
        list_push_back(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, size));
    return tokens;
}

size_t _regex_run(_regex_t *regex, const char *str) {
    if(REGEX_COMPILED != regex->state)
        assert(0 == "A regex cannot be run without first being compiled.");
    List(char) *str_list = list_new(char);
    size_t size = strlen(str);
    for(size_t i = 0; i < size; ++i) {
        list_push_back(str_list, str[i]);
    }
    size_t accept_flag = call(regex->fdfa, list_get_iterator(str_list));
    list_free(str_list);
    return accept_flag;
}

// LINEAR SCAN VERSION: [start, end)
_regex_match_t _regex_find_left_most_match_linear_scan(_regex_t *regex, const char *str, size_t start, size_t end) {
    const _regex_match_t inv_match = {-1,-1};
    const _regex_match_t zero_match = {start, 0};
    List(char) *str_list = list_new(char);
    size_t size = strlen(str);
    size_t right_bound = -1;
    if (1 == call(regex->fdfa, list_get_iterator(str_list)))
        return zero_match;
    for(size_t i = start; i < end; ++i) {
        list_push_back(str_list, str[i]);
        if(1 == call(regex->fdfa, list_get_iterator(str_list))) {
            right_bound = i; // Indicates a match within [start, i].
            break;
        }
    }
    list_free(str_list);

    if (right_bound == -1) return inv_match;

    str_list = list_new(char);
    size_t left_bound = -1;
    for(size_t i = right_bound; i >= start; --i) {
        list_push_front(str_list, str[i]); // PUSH FRONT!
        if(1 == call(regex->fdfa, list_get_iterator(str_list))) {
            left_bound = i;
            break;
        }
    }
    list_free(str_list);

    if (left_bound == -1) return inv_match;

    _regex_match_t leftmost_match = {left_bound, right_bound - left_bound + 1};
    return leftmost_match;
}

List(char)* _regex_construct_list_with_interval(const char *str, size_t start, size_t end) {
    List(char) *str_list = list_new(char);
    for(size_t i = start; i < end; ++i) {
        list_push_back(str_list, str[i]);
    }
    return str_list;
}


List(size_t)* _regex_get_mids(size_t size, size_t start, size_t end) {
    List(size_t) *mids = list_new(size_t);
    for(size_t i = 0; i < size - 1; ++i) {
        list_push_back(mids, ((size - i - 1) * start + (i + 1) * end) / size);
    }
    return mids;
}

_regex_match_t _regex_find_left_most_match_binary_search(_regex_t *regex, const char *str, size_t start, size_t end, size_t search_chunk_size) {
    const _regex_match_t inv_match = {-1,-1};
    const _regex_match_t zero_match = {start, 0};
    size_t size = strlen(str);
    size_t right_bound = end;
    List(char) *str_list;

    // Binary search for right_bound where: [start, right_bound) == 0
    size_t left = start; size_t right = right_bound;
    while(1 < right - left) {
        size_t optimal_size = max(2, (right - left) / search_chunk_size);
        List(size_t) *mids = _regex_get_mids(optimal_size, left, right);
        while(list_size(mids)) {
            size_t mid = list_get_front(mids); list_pop_front(mids);
            str_list = _regex_construct_list_with_interval(str, start, mid);
            size_t result = call(regex->fdfa, list_get_iterator(str_list));
            0 == result ? (left = mid) : (right = mid);
            list_free(str_list);
            if (1 == result)
                break;
        }
        list_free(mids);
    }
    right_bound = left;

    if (right_bound == -1 || right_bound == end - 1) return inv_match;

    str_list = list_new(char);
    size_t left_bound = -1;
    for(size_t i = right_bound; i >= start; --i) {
        list_push_front(str_list, str[i]); // PUSH FRONT!
        if(1 == call(regex->fdfa, list_get_iterator(str_list))) {
            left_bound = i;
            break;
        }
    }
    list_free(str_list);

    if (left_bound == -1) return inv_match;

    _regex_match_t leftmost_match = {left_bound, right_bound - left_bound + 1};
    return leftmost_match;
}

_regex_match_t _regex_find_left_most_match_full_binary_search(_regex_t *regex, const char *str, size_t start, size_t end) {
    const _regex_match_t inv_match = {-1,-1};
    const _regex_match_t zero_match = {start, 0};
    size_t size = strlen(str);
    List(char) *str_list;

    // Binary search for right_bound where: [start, right_bound) == 0
    size_t left = start; size_t right = end;
    while(1 < right - left) {
        size_t optimal_size = max(2, (right - left) / SEARCH_CHUNK_SIZE);
        List(size_t) *mids = _regex_get_mids(optimal_size, left, right);
        while(list_size(mids)) {
            size_t mid = list_get_front(mids); list_pop_front(mids);
            str_list = _regex_construct_list_with_interval(str, start, mid);
            size_t result = call(regex->fdfa, list_get_iterator(str_list));
            0 == result ? (left = mid) : (right = mid);
            list_free(str_list);
            if (1 == result)
                break;
        }
        list_free(mids);
    }
    size_t right_bound = left;

    if (right_bound == -1 || right_bound == end - 1) return inv_match;

    // binary search for [left_bound, right_bound] == 0
    left = start; right = right_bound + 1;
    while(1 < right - left) {
        size_t optimal_size = max(2, (right - left) / TOKEN_CHUNK_SIZE);
        List(size_t) *mids = _regex_get_mids(optimal_size, left, right);
        while(list_size(mids)) {
            size_t mid = list_get_back(mids); list_pop_back(mids); // NOTE: pop_back vs pop_front
            str_list = _regex_construct_list_with_interval(str, mid, right_bound+1); // [mid, rb)
            size_t result = call(regex->fdfa, list_get_iterator(str_list));
            0 != result ? (left = mid) : (right = mid);
            list_free(str_list);
            if (1 == result)
                break;
        }
        list_free(mids);
    }
    size_t left_bound = left;

    if (left_bound == -1) return inv_match;

    _regex_match_t leftmost_match = {left_bound, right_bound - left_bound + 1};
    return leftmost_match;
}


List(_regex_match_t)* _regex_find_all_regex_matches(_regex_t *regex, const char *str) {
    if(REGEX_COMPILED != regex->state)
        assert(0 == "A regex cannot be run without first being compiled.");
    List(_regex_match_t) *matches = list_new(_regex_match_t);
    List(char) *str_list = list_new(char);
    size_t size = strlen(str) + 1;
    _regex_match_t next_match = {0, 0};
    next_match = _regex_find_left_most_match_binary_search(regex, str, 0, size, SEARCH_CHUNK_SIZE);
    while(next_match.begin != -1 && next_match.length != -1) { // a -1 for both start and end indicates a non-match
        list_push_back(matches, next_match);
        if(next_match.length == 0) // break if it is a zero length match
            break;
        next_match = _regex_find_left_most_match_binary_search(regex, str, next_match.begin + 1, size, SEARCH_CHUNK_SIZE);

        /**
         * For "adaptive search" use "full_binary_search" that uses bin_search both ways. Otherwise, for most cases, 
         * a single left-binary-search is faster. Adaptive search finds the left-bound using binary search, but it is
         * more inefficient because the regex `run' function runs very fast (so the effects of spamming `run' is negligible).
         * 
         * next_match = (TOKEN_CHUNK_SIZE < next_match.length) ? _regex_find_left_most_match_full_binary_search(regex, str, next_match.begin + 1, size)
         *                                                     : _regex_find_left_most_match_binary_search(regex, str, next_match.begin + 1, size);
        */
    }
    return matches;
}

List(_regex_match_t)* _regex_find_all_regex_matches_exponential_pdf(_regex_t *regex, const char *str) {
    if(REGEX_COMPILED != regex->state)
        assert(0 == "A regex cannot be run without first being compiled.");
    List(_regex_match_t) *matches = list_new(_regex_match_t);
    List(char) *str_list = list_new(char);
    size_t size = strlen(str) + 1;
    _regex_match_t next_match = {0, 0};
    next_match = _regex_find_left_most_match_binary_search(regex, str, 0, size, SEARCH_CHUNK_SIZE);
    size_t num_deltas = 0;      // number of collected intervals
    size_t sum_of_deltas = 0;   // sum of intervals.
    // MLE = (n - 2) / SUM(X)
    while(next_match.begin != -1 && next_match.length != -1) { // a -1 for both start and end indicates a non-match
        list_push_back(matches, next_match);
        if(next_match.length == 0) // break if it is a zero length match
            break;
        
        size_t prev_end = next_match.begin + 1;
        size_t predicted_chunk_size = (2 < num_deltas && sum_of_deltas / (num_deltas - 2) < SEARCH_CHUNK_SIZE)
            ? sum_of_deltas / (num_deltas - 2)
            : SEARCH_CHUNK_SIZE;
        // if(num_deltas % 50 == 0) printf("predicted_chunk_size: %zu\n", predicted_chunk_size);
        next_match = _regex_find_left_most_match_binary_search(regex, str, next_match.begin + 1, size, 
            predicted_chunk_size);
        num_deltas += 1;
        sum_of_deltas += (next_match.begin + next_match.length - prev_end);

        /**
         * For "adaptive search" use "full_binary_search" that uses bin_search both ways. Otherwise, for most cases, 
         * a single left-binary-search is faster. Adaptive search finds the left-bound using binary search, but it is
         * more inefficient because the regex `run' function runs very fast (so the effects of spamming `run' is negligible).
         * 
         * next_match = (TOKEN_CHUNK_SIZE < next_match.length) ? _regex_find_left_most_match_full_binary_search(regex, str, next_match.begin + 1, size)
         *                                                     : _regex_find_left_most_match_binary_search(regex, str, next_match.begin + 1, size);
        */
    }
    return matches;
}

void _regex_destroy(_regex_t *regex) {
    free((void*) regex->raw_regex);
    nfa_free(regex->nfa);
    fdfa_free(regex->fdfa);
    free(regex);
}

_regex_fns_t _regex_fn_impl_ = {
    &_regex_from,
    &_regex_compile,
    &_regex_run,
    // &_regex_find_all_regex_matches,
    &_regex_find_all_regex_matches_exponential_pdf,
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