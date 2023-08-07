#include "regex.h"
// #include "lexer.h" // for syntax-completion

define_vector(_token_rule_t);

typedef List(_regex_match_t)* _matches_ptr;
define_vector(_matches_ptr);

TokenRules* _token_rules_new() {
    TokenRules *new_tr = (TokenRules*) malloc(sizeof(TokenRules));
    new_tr->rules = vector_new(_token_rule_t);
    return new_tr;
}

static void _token_rules_free(TokenRules *tr) {
    size_t size = vector_size(tr->rules);
    for(size_t i = 0; i < size; ++i) {
        regex_free(vector_get(tr->rules, i).regex);
    }
    vector_free(tr->rules);
    free(tr);
}

void _token_rules_add_rule(TokenRules *tr, const char *name, size_t pre, size_t post, const char *raw_regex) {
    _token_rule_t new_tr_instance = {name, pre, post, regex_from(raw_regex)};
    vector_push_back(tr->rules, new_tr_instance);
}

void _token_rules_compile(TokenRules *tr) {
    size_t size = vector_size(tr->rules);
    for(size_t i = 0; i < size; ++i) {
        regex_compile(vector_get(tr->rules, i).regex);
    }
}

size_t _token_rules_matches_vector_has_matches(Vector(_matches_ptr) *matches);

List(_token_t)* _token_rules_tokenize(TokenRules *tr, const char *input) {
    Vector(_matches_ptr) *matches = vector_new(_matches_ptr);
    vector_resize_val(matches, vector_size(tr->rules), NULL);
    size_t size = vector_size(tr->rules);
    for(size_t i = 0; i < size; ++i) {
        vector_set(matches, i, regex_find_all(vector_get(tr->rules, i).regex, input));
        // printf("Finished lexing! %zu/%zu\n", i + 1, size);
    }
    List(_token_t) *tokens = list_new(_token_t); size_t min_beginning = 0;
    while(_token_rules_matches_vector_has_matches(matches)) {
        size_t min_ind = 0; size_t min_val = 0UL - 1; 
        for(size_t i = 0; i < size; ++i) {
            _matches_ptr next_match = vector_get(matches, i);
            // ----
            // TODO delete the following block:
            // char buf[500];
            // ----
            while(0 < list_size(next_match) && 
                (list_get_front(next_match).begin + vector_get(tr->rules, i).pre_offset < min_beginning)) {
                    // ----
                    // TODO delete the following block:
                    // const char* beg = input + list_get_front(next_match).begin + vector_get(tr->rules, i).pre_offset;
                    // size_t sz = list_get_front(next_match).length - vector_get(tr->rules, i).pre_offset - vector_get(tr->rules, i).post_offset;
                    // memcpy(buf, beg, sz);
                    // buf[sz] = '\0';
                    // printf("discarding: \"%s\"\n", buf);
                    // ----
                    list_pop_front(next_match);
            }
            if(0 < list_size(next_match) && min_val > list_get_front(next_match).begin + vector_get(tr->rules, i).pre_offset)
                (min_ind = i, min_val = list_get_front(next_match).begin + vector_get(tr->rules, i).pre_offset);
        }
        if (min_val == 0UL - 1) // If the minimum value has not changed.
            break;
        const char* token_ptr = input + list_get_front(vector_get(matches, min_ind)).begin + vector_get(tr->rules, min_ind).pre_offset;
        size_t token_length = list_get_front(vector_get(matches, min_ind)).length - vector_get(tr->rules, min_ind).pre_offset - vector_get(tr->rules, min_ind).post_offset;
        _token_t next_token = {
            vector_get(tr->rules, min_ind).name, 
            token_ptr,
            token_length
        };
        min_beginning = list_get_front(vector_get(matches, min_ind)).begin + list_get_front(vector_get(matches, min_ind)).length - vector_get(tr->rules, min_ind).post_offset;
        list_pop_front(vector_get(matches, min_ind));
        // if(0 == list_size(tokens) || list_get_back(tokens).ptr + list_get_back(tokens).length <= next_token.ptr)
        list_push_back(tokens, next_token);
    }
    return tokens;
}

size_t _token_rules_matches_vector_has_matches(Vector(_matches_ptr) *matches) {
    for(size_t i = 0; i < vector_size(matches); ++i)
        if(0 < list_size(vector_get(matches, i)))
            return 1;
    return 0;
}

_token_rules_fns_t _token_rules_fns_impl = {
    &_token_rules_new,
    &_token_rules_free,
    &_token_rules_add_rule,
    &_token_rules_compile,
    &_token_rules_tokenize
};