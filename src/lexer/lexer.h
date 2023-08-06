/**
 * @file lexer.h
 * @author Siddhant Mane (siddhantmane@gmail.com)
 * @brief A single include header file for lexing.
 * @version 0.1
 * @date 2023-07-18
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef LEXER_H
#define LEXER_H

#include "regex.h"

/**
 * Token rules are defined as a mapping from a "token name" (const char*)
 * to a Regex that accepts inputs for that specific language. 
 * 
 */

#define TokenRules                                                   _token_rules_t
#define Token                                                        _token_t
#define token_rules_new()                                            (_token_rules_fns_impl._new())
#define token_rules_free(tr)                                         (_token_rules_fns_impl._free((tr)))
#define token_rules_add_rule(tr, name, raw_regex)                    (_token_rules_fns_impl._add_rule((tr), (name), 0, 0, (raw_regex)))
#define token_rules_add_rule_offset(tr, name, pre, post, raw_regex)  (_token_rules_fns_impl._add_rule((tr), (name), (pre), (post), (raw_regex)))
#define token_rules_compile(tr)                                      (_token_rules_fns_impl._compile((tr)))
#define token_rules_tokenize(tr, input)                              (_token_rules_fns_impl._tokenize((tr), (input)))

struct _token_rule_ {
    const char *name;
    size_t pre_offset;
    size_t post_offset;
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

// Non-standard, but necessary to require the definition of List(_token_t).
#include "lexer_def.c"

struct _token_rules_fns_ {
    TokenRules* (*_new)();
    void (*_free)(TokenRules*);
    void (*_add_rule)(TokenRules*, const char*, size_t, size_t, const char*);
    void (*_compile)(TokenRules*);
    List(_token_t)* (*_tokenize)(TokenRules*, const char*);
};
typedef struct _token_rules_fns_ _token_rules_fns_t;

extern _token_rules_fns_t _token_rules_fns_impl;

#include "lexer.c"

#endif