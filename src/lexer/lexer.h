#ifndef LEXER_H
#define LEXER_H

#include "regex.h"

/**
 * Token rules are defined as a mapping from a "token name" (const char*)
 * to a Regex that accepts inputs for that specific language. 
 * 
 * DFA's that accept a specific language can be encoded as functions that 
 * take in a string (const char*) and return `1` if it accepts or `0` if 
 * it rejects. In order to create a list of tokens from a given string, a 
 * binary-search-like algorithm with a DFA function that accepts if the string
 * **contains** the given token can be used. 
 * 
 * In order to avoid confusion, I will be separating the `function' representation
 * of a DFA from an interal (map-based) representation of a DFA
 *   - An `fdfa_t' is a dfa represented as a function closure (or a C-like representation of a 
 *     function closure)
 *     - TYPE : [context] st.
 *       - call(context, const char*)   ->   size_t (indicating true[1] / false[0])
 *   - A `dfa_t' is a dfa represented through a directed graph. The internal representation
 *     of a `dfa_t' isn't important, because a function that converts a `dfa_t' to a
 *     `fdfa_t' must be supplied.
 * 
 * 
 */

#define TokenRules                                  _token_rules_t
#define tr_new()                                    (_token_rules_fns_impl._new())
#define tr_free(tr)                                 (_token_rules_fns_impl._free((tr)))
#define tr_add_rule(tr, name, offset, raw_regex)    (_token_rules_fns_impl._add_rule((tr), (name), (offset), (raw_regex)))
#define tr_compile(tr)                              (_token_rules_fns_impl._compile((tr)))
#define tr_tokenize(tr, input)                      (_token_rules_fns_impl._tokenize((tr), (input)))

struct _token_rule_ {
    const char *name;
    size_t offset;
    Regex *regex;
};
typedef struct _token_rule_ _token_rule_t;

/**
 * A `_token_rules_' struct contains a vector of 
 */
typedef struct __token_rule_t_vector_ __token_rule_t_vector_t;
struct _token_rules_ {
    Vector(_token_rule_t) *rules;
};
typedef struct _token_rules_ _token_rules_t;

struct _token_ {
    const char *name;
    const char *ptr;
    size_t length;
};
typedef struct _token_ _token_t;

typedef struct __token_t_list_ _token_t_list_t;
struct _token_rules_fns_ {
    TokenRules* (*_new)();
    void (*_free)(TokenRules*);
    void (*_add_rule)(TokenRules*, const char*, size_t, const char*);
    void (*_compile)(TokenRules*);
    List(_token_t)* (*_tokenize)(TokenRules*);
};
typedef struct _token_rules_fns_ _token_rules_fns_t;

extern _token_rules_fns_t _token_rules_fns_impl;

#endif