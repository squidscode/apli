#ifndef APLI_H
#define APLI_H

#include "parser/parser.h"
#include "util/macro_magic.h"

/**
 * The following macros act describe the public-facing API calls the user can make
 * to construct a compiler / interpreter. 
 *
 * The user will write apli_functions in order to create "evaluation hooks" for 
 * specific EBNF shapes.
 *
 *
 */


#define apli_function(lhs_terminal_name) \
    apli_return_type _apli_##lhs_terminal_name##_eval_hook(_parse_tree_node_t node)
#define apli_fn(lhs_terminal_name)      apli_function(lhs_terminal_name)
#define apli_ebnf_rule(lhs_terminal_name, ...) \
    ebnf_rules_add_rule(ebnf_rules, ebnf_rule_from(lhs_terminal_name, __VA_ARGS__))
#define STR_BUFFER_SIZE 100

#define apli_define_non_terminal(name) \
    Terminal name = non_terminal_from(#name); \
    map_insert(eval_fns, #name, &_apli_##name##_eval_hook)

#define apli_define_terminal(name)      Terminal name = terminal_from(#name)
#define SEMI_COLON()                    ;
#define apli_non_terminals(...)         MAP(apli_define_non_terminal, SEMI_COLON, __VA_ARGS__)
#define apli_terminals(...)             MAP(apli_define_terminal, SEMI_COLON, __VA_ARGS__)
#define apli_define_functions(...)      MAP(apli_function, SEMI_COLON, __VA_ARGS__)

#define _STRINGIFY(str)         #str
#define STRINGIFY(str)          DEFER3(_STRINGIFY)(str)
#define THIRD(a,b,c,...) c
#define FORTH(a,b,c,d,...) d
#define apli_define_regex(expr) \
    DEFER2(IF_ELSE) (DEFER1(HAS_ARGS) DEFER1(_REST) DEFER1(_REST) expr) ( \
        token_rules_add_rule_offset(token_rules, STRINGIFY(DEFER3(FIRST) expr), DEFER3(THIRD) expr, DEFER3(FORTH) expr, DEFER3(SECOND) expr), \
        token_rules_add_rule(token_rules, STRINGIFY(DEFER3(FIRST) expr), DEFER3(SECOND) expr) \
    )

#define apli_regex(...)             MAP(apli_define_regex, SEMI_COLON, __VA_ARGS__)

#define apli_regex_compile()        token_rules_compile(token_rules)

#define __APLI_START__ \
    int main(int argc, char **argv) { \
        apli_main_init();

#define apli_main_init() \
    EbnfRules *ebnf_rules = ebnf_rules_new(); \
    TokenRules *token_rules = token_rules_new(); \
    _parse_tree_t parse_tree_result; \
    eval_fns = map_new(apli_function_name, apli_function_reference); \
    map_set_key_eq(eval_fns, &str_eq); \
    map_set_hash(eval_fns, &str_hash); \
    char buf[STR_BUFFER_SIZE]

#define __APLI_END__              }


size_t str_eq(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
}

size_t str_hash(const char* str) {
    size_t hash = 0;
    char *ptr = (char*) ((void*) &str);
    size_t i = 0;
    size_t mod = sizeof(size_t) / sizeof(char);
    size_t offset = 0;
    while(ptr[i] != '\0') {
        hash ^= ((255UL & ptr[i++]) << (8 * offset++));
        offset %= mod;
    }
    return hash;
}

typedef const char* apli_function_name;
#define apli_init() \
    typedef apli_return_type (*apli_function_reference)(_parse_tree_node_t); \
    define_map(apli_function_name, apli_function_reference); \
    Map(apli_function_name, apli_function_reference) *eval_fns

#define apli_evaluate(input) \
    (parse_tree_result = apli_get_parse_tree(input), \
     apli_evaluate_node(parse_tree_result.root))

#define apli_evaluate_node(node) \
    map_at(eval_fns, \
        node.root.is_terminal_t \
        ? (assert(node.root.ptr.terminal.name[node.root.ptr.terminal.name_length] == '\0'), node.root.ptr.terminal.name) \
        : node.root.ptr.token.name)(node)

#define apli_get_parse_tree(input) \
    ebnf_rules_construct_parse_tree(ebnf_rules, token_rules_tokenize(token_rules, input))

#endif