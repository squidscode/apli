#include "regex.h"
#include "dfa.h"
#include "nfa.h"
#include <stdio.h>

init_regex();

typedef enum {RAW_LOADED, COMPILED} _regex_state_type;
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
    new_regex->state = RAW_LOADED;
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
size_t _regex_expand_token(Nfa(size_t, char)*, size_t, const char*, size_t);
size_t _regex_string_segment_equals(const char*, _regex_string_segment_t segment);
void _debug_print_regex_string_segment_t(_regex_string_segment_t rss);

_regex_t* _regex_compile(_regex_t *regex) {
    size_t regex_size = strlen(regex->raw_regex);
    regex->nfa = nfa_new(size_t, char, 0);
    size_t end = _regex_parse(regex->nfa, 0, regex->raw_regex, regex_size);
    printf("start : %zu\nend : %zu\n", 0UL, end);
    nfa_add_accept_state(regex->nfa, end);
    Set(char) *alphabet = set_new(char);
    for(int i = 0; i < 128; ++i) {
        set_insert(alphabet, i);
    }
    Dfa(size_t_set_ptr_t, char) *dfa = nfa_to_dfa(regex->nfa, alphabet);
    regex->fdfa = dfa_to_fdfa(dfa);
    regex->state = COMPILED;
    return regex;
}

/* TODO the following:
 * - add '^' and '$' special characters
 * - parse '{}' family correctly.
 * - compress the final dfa created AND free the original set nfa.
 */

size_t _regex_parse(Nfa(size_t, char) *nfa, size_t begin_expansion_state,
    const char* raw_regex, size_t raw_regex_size) {
    List(_regex_string_segment_t) *capturing_groups = _regex_split_by_capturing_groups(raw_regex, raw_regex_size);
    List(size_t) *capturing_groups_ends = list_new(size_t);
    printf("capturing groups size : %zu\n", list_size(capturing_groups));
    size_t next_start = begin_expansion_state + 1;
    size_t end = begin_expansion_state - 1;
    while(0 < list_size(capturing_groups)) {
        _regex_string_segment_t segment = list_get_front(capturing_groups);
        _debug_print_regex_string_segment_t(segment);
        nfa_add_epsilon_transition(nfa, begin_expansion_state, next_start);
        list_push_back(capturing_groups_ends,
            (end = _regex_expand(nfa, next_start, segment.ptr, segment.length)));
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
        if('(' == ptr[i])
            paren_level += 1;
        if(')' == ptr[i])
            paren_level -= 1;
        if('|' == ptr[i] && 0 == paren_level) {
            list_push_back(groups, _regex_string_segment_from(ptr, capture_group_begin_offset, i));
            capture_group_begin_offset = i + 1;
        }
    }
    if (0 != paren_level)
        assert(0 == "Incorrectly formatted regex.");
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
    for(int i = 0; i < segment.length; ++i) {
        if(str[i] != segment.ptr[i])
            return 0;
    }
    return 1;
}

size_t _regex_expand(Nfa(size_t, char) *nfa, size_t begin_expansion_state, const char* raw_regex, size_t raw_regex_size) {
    /* the raw_regex string must not have multiple segments (ie. there are no level 0 alternation signs). */
    List(_regex_string_segment_t) *tokens = _regex_split_by_tokens(raw_regex, raw_regex_size);
    size_t next_start = begin_expansion_state;
    size_t end = begin_expansion_state - 1;
    /* The regex token parser uses 1 look-ahead. */
    _regex_string_segment_t current = list_get_front(tokens);
    printf("[expand]: "); _debug_print_regex_string_segment_t(current);
    list_pop_front(tokens);
    while (0 < list_size(tokens)) {
        _regex_string_segment_t next = list_get_front(tokens);
        printf("[expand]: "); _debug_print_regex_string_segment_t(next);
        if(_regex_string_segment_equals("*", current) || _regex_string_segment_equals("?", current)
            || _regex_string_segment_equals("+", current)) {
        } else if (_regex_string_segment_equals("*", next)) {
            nfa_add_epsilon_transition(nfa, next_start, next_start+1);
            end = _regex_expand_token(nfa, next_start+1, current.ptr, current.length);
            nfa_add_epsilon_transition(nfa, end, next_start + 1);
            nfa_add_epsilon_transition(nfa, end, end+1);
            nfa_add_epsilon_transition(nfa, next_start, end+1);
            end = end + 1;
            next_start = end;
        } else if (_regex_string_segment_equals("?", next)) {
            nfa_add_epsilon_transition(nfa, next_start, next_start+1);
            end = _regex_expand_token(nfa, next_start+1, current.ptr, current.length);
            nfa_add_epsilon_transition(nfa, end, end+1);
            nfa_add_epsilon_transition(nfa, next_start, end+1);
            end = end + 1;
            next_start = end;
        } else if (_regex_string_segment_equals("+", next)) {
            nfa_add_epsilon_transition(nfa, next_start, next_start+1);
            end = _regex_expand_token(nfa, next_start+1, current.ptr, current.length);
            nfa_add_epsilon_transition(nfa, end, end+1);
            nfa_add_epsilon_transition(nfa, end, next_start + 1);
            end = end + 1;
            next_start = end;
        } else {
            end = _regex_expand_token(nfa, next_start, current.ptr, current.length);
            next_start = end;
        }
        current = next;
        list_pop_front(tokens);
    }
    /* TODO repition of this boolean !! */
    if(_regex_string_segment_equals("*", current) || _regex_string_segment_equals("?", current)
            || _regex_string_segment_equals("+", current)) {
    } else {
        end = _regex_expand_token(nfa, next_start, current.ptr, current.length);
        next_start = end;
    }
    return end;
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
        if('r' == raw_regex[1])
            nfa_add_transition(nfa, begin_expansion_state, '\r', begin_expansion_state + 1);
        if('t' == raw_regex[1])
            nfa_add_transition(nfa, begin_expansion_state, '\t', begin_expansion_state + 1);
        return begin_expansion_state + 1;
    } else if ('(' == raw_regex[0] && ')' == raw_regex[raw_regex_size - 1]) {
        return _regex_parse(nfa, begin_expansion_state, &raw_regex[1], raw_regex_size - 2);
    } else if ('[' == raw_regex[0] && ']' == raw_regex[raw_regex_size - 1]) {
        for(size_t ind = 1; ind < raw_regex_size - 1; ++ind) {
            nfa_add_transition(nfa, begin_expansion_state, raw_regex[ind], begin_expansion_state + 1);
        }
        return begin_expansion_state + 1;
    } else if ('{' == raw_regex[0] && '}' == raw_regex[raw_regex_size - 1]) {
        /* TODO implement this!! */
        return begin_expansion_state;
    }
    _regex_string_segment_t rss = {raw_regex, raw_regex_size};
    _debug_print_regex_string_segment_t(rss);
    assert(0 == "Invalid regex format.");
}

/* Invariant: the size of the list that is returned is at least 1. */
List(_regex_string_segment_t)* _regex_split_by_tokens(const char *ptr, size_t size) {
    List(_regex_string_segment_t) *tokens = list_new(_regex_string_segment_t);
    size_t paren_level = 0; size_t bracket_level = 0; size_t curly_bracket_level = 0;
    size_t capture_group_begin_offset = 0;
    for(size_t i = 0; i < size; ++i) {
        if ('(' != ptr[i] && ')' != ptr[i] && 0 == paren_level
            && '[' != ptr[i] && ']' != ptr[i] && 0 == bracket_level
            && '{' != ptr[i] && '}' != ptr[i]) {
            list_push_back(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, i+1));
            capture_group_begin_offset = i + 1;
        } else if ('\\' == ptr[i] && 0 == paren_level && 0 == bracket_level) {
            i += 1;
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
        if (1 < bracket_level || 0 > bracket_level
            || 0 > paren_level
            || 1 < curly_bracket_level || 0 > curly_bracket_level)
            assert(0 == "Incorrectly formatted regex.");
    }
    if (0 != paren_level || 0 != bracket_level)
        assert(0 == "Incorrectly formatted regex.");
    if (size != capture_group_begin_offset)
        list_push_back(tokens, _regex_string_segment_from(ptr, capture_group_begin_offset, size));
    return tokens;
}

size_t _regex_run(_regex_t *regex, const char *str) {
    if(COMPILED != regex->state)
        assert(0 == "A regex cannot be run without first being compiled.");
    List(char) *str_list = list_new(char);
    size_t size = strlen(str);
    for(size_t i = 0; i < size; ++i) {
        list_push_back(str_list, str[i]);
    }
    size_t accept_flag = call(regex->fdfa, get_iterator(str_list));
    list_free(str_list);
    return accept_flag;
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
    &_regex_destroy
};

void _debug_print_regex_string_segment_t(_regex_string_segment_t rss) {
    char *buf = (char*) (malloc(rss.length + 1));
    for(size_t i = 0; i < rss.length; ++i)
        buf[i] = rss.ptr[i];
    buf[rss.length] = '\0';
    printf("String Segment: \"%s\"\n", buf);
    free(buf);
}