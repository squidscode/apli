#include "lexer.h"

TokenRules* _token_rules_new() {

}

void _token_rules_free(TokenRules* tr) {

}

void _token_rules_add_rule(TokenRules* tr, const char* name, size_t offset, const char* raw_regex) {

}

void _token_rules_compile(TokenRules* tr) {

}

List(_token_t)* _token_rules_tokenize(TokenRules* tr) {

}

_token_rules_fns_t _token_rules_fns_impl = {
    &_token_rules_new,
    &_token_rules_free,
    &_token_rules_add_rule,
    &_token_rules_compile,
    &_token_rules_tokenize
};