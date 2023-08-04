#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include <string.h>
#include <stdarg.h>

/**
 * 
 * --- Implementation Details / Blueprint ---
 * LHLR parser implementation. Given a ebnf rules, in order of precedence:
 *  (1) Finds the minimum look-ahead needed to generate a deterministic parser
 *  (2) Construct a reduce rule for each of the ebnf rules.
 *  (3) Using the look-ahead value, create a map(list_of_char -> shift/reduce rule).
 *      in cases where the look-ahead does not exist (expressed as a list_of_chars with
 *      size < look-ahead), then we pick the reduce rule that corresponds to the highest
 *      precedence rule that fits.
 *  (4) If the parsing step results in an error, then we change the input argument that corresponds
 *      to the status of the parser and return NULL as the parse tree.
 *      OTHERWISE, in the case of successful compilation, we can return a `success' status and a 
 *      pointer to the parse tree/forest.
 *
 */

#define Terminal                                    _terminal_t
#define terminal_from(name)                         (_terminal_from(1, (name), strlen((name))))
#define non_terminal_from(name)                     (_terminal_from(0, (name), strlen((name))))
#define terminal_from_segment(name, length)         (_terminal_from(1, (name), (length)))
#define non_terminal_from_segment(name, length)     (_terminal_from(0, (name), (length)))
#define EbnfRules                                   _ebnf_rules_t
#define ebnf_rules_new()                            (_ebnf_rules_new())
#define ebnf_rules_add_rule(ebnf_rules, ebnf_rule)  (_ebnf_rules_fn_impl._add_rule((ebnf_rules), (ebnf_rule)))
#define ebnf_rule_from(lhs, ...)                    (_ebnf_rule_from((lhs), PP_NARG(__VA_ARGS__), __VA_ARGS__))
#define ebnf_rule_from_vector(lhs, rule_vec)        (_ebnf_rule_from_vec((lhs), (rule_vec)))
#define min(x,y)                                    (((x) < (y)) ? (x) : (y))
#define max(x,y)                                    (((x) < (y)) ? (y) : (x))

// Marco magic! This macro is not mine, it's from stackoverflow
#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_128TH_ARG(__VA_ARGS__)
#define PP_128TH_ARG( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,_64,_65,_66,_67,_68,_69,_70, \
         _71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
         _81,_82,_83,_84,_85,_86,_87,_88,_89,_90, \
         _91,_92,_93,_94,_95,_96,_97,_98,_99,_100, \
         _101,_102,_103,_104,_105,_106,_107,_108,_109,_110, \
         _111,_112,_113,_114,_115,_116,_117,_118,_119,_120, \
         _121,_122,_123,_124,_125,_126,_127,N,...) N
#define PP_RSEQ_N() \
         127,126,125,124,123,122,121,120, \
         119,118,117,116,115,114,113,112,111,110, \
         109,108,107,106,105,104,103,102,101,100, \
         99,98,97,96,95,94,93,92,91,90, \
         89,88,87,86,85,84,83,82,81,80, \
         79,78,77,76,75,74,73,72,71,70, \
         69,68,67,66,65,64,63,62,61,60, \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

/**
 * A non-terminal is a symbol that is a place holder for other terminal/non-terminal symbols.
 * A terminal is an "elementary" symbol. In this case, each terminal must correspond to a specific token.
 */
struct _terminal_ {
    char is_terminal : 1;
    const char *name;
    size_t name_length;
} __attribute__((packed));
typedef struct _terminal_ _terminal_t; 

static const _terminal_t null_terminal = {-1, "", 0};

_terminal_t _terminal_from(char is_terminal, const char *name, size_t name_length) {
    _terminal_t term = {is_terminal, name, name_length};
    return term;
}

define_vector(_terminal_t);

struct _ebnf_rule_ {
    _terminal_t lhs_terminal;
    Vector(_terminal_t) *rule;
};
typedef struct _ebnf_rule_ _ebnf_rule_t;

define_vector(_ebnf_rule_t);
typedef void* _void_ptr_;
define_map(_terminal_t, _void_ptr_);
typedef Map(_terminal_t, _void_ptr_) _terminal_tree_t;
define_map(_terminal_t, size_t);

struct _ebnf_rules_ {
    Vector(_ebnf_rule_t) *rules;
};
typedef struct _ebnf_rules_ _ebnf_rules_t;

union _terminal_or_token_ptr_ {
    _terminal_t *terminal;
    _token_t *token;
};
typedef union _terminal_or_token_ptr_ _terminal_or_token_ptr_t;

struct _parse_tree_value_ {
    char is_terminal : 1;
    _terminal_or_token_ptr_t ptr;
};
typedef struct _parse_tree_value_ _parse_tree_value_t;
define_vector(_parse_tree_value_t);

struct _parse_tree_node_ {
    _parse_tree_value_t root;
    Vector(_parse_tree_value_t) *children;
};
typedef struct _parse_tree_node_ _parse_tree_node_t;

struct _parse_tree_ {
    _parse_tree_node_t root;
};
typedef struct _parse_tree_ _parse_tree_t;

struct _ebnf_rules_fn_ {
    _ebnf_rules_t* (*_new)();
    void (*_add_rule)(_ebnf_rules_t*, _ebnf_rule_t);
    _parse_tree_t (*_construct_parse_tree)(_ebnf_rules_t*, List(_token_t)*);
};
typedef struct _ebnf_rules_fn_ _ebnf_rules_fn_t;

extern _ebnf_rules_fn_t _ebnf_rules_fn_impl;
EbnfRules* _ebnf_rules_new();
void _ebnf_rules_add_rule(_ebnf_rules_t *ebnf_rules, _ebnf_rule_t rule);
static size_t _ebnf_rules_find_minimum_lookahead(_ebnf_rules_t *ebnf_rules);
static size_t _ebnf_rule_index_of_left_most_difference(_ebnf_rule_t rule1, _ebnf_rule_t rule2);
static size_t _terminal_equals(_terminal_t terminal1, _terminal_t terminal2);
static _terminal_tree_t *_ebnf_rules_construct_terminal_tree(_ebnf_rules_t*, size_t);

EbnfRules* _ebnf_rules_new() {
    EbnfRules *ebnf_rules = (EbnfRules*) malloc(sizeof(EbnfRules));
    ebnf_rules->rules = vector_new(_ebnf_rule_t);
    return ebnf_rules;
}

void _ebnf_rules_add_rule(_ebnf_rules_t *ebnf_rules, _ebnf_rule_t rule) {
    vector_push_back(ebnf_rules->rules, rule);
}

_ebnf_rule_t _ebnf_rule_from(_terminal_t lhs, size_t num_va_args, ...) {
    va_list va_args;
    Vector(_terminal_t) *new_rule = vector_new(_terminal_t);
    va_start(va_args, num_va_args);
    for(size_t i = 0; i < num_va_args; ++i) {
        vector_push_back(new_rule, va_arg(va_args, _terminal_t));
    }
    va_end(va_args);

    _ebnf_rule_t new_ebnf_rule = {lhs, new_rule};
    return new_ebnf_rule;
}

_parse_tree_t _ebnf_construct_parse_tree(_ebnf_rules_t *rules, List(_token_t) *token_list) {
    size_t minimum_lookahead = _ebnf_rules_find_minimum_lookahead(rules);
    _terminal_tree_t *terminal_tree = _ebnf_rules_construct_terminal_tree(rules, minimum_lookahead);
    // begin shift-reduce with terminal_tree:

}

static size_t _ebnf_rules_find_minimum_lookahead(_ebnf_rules_t *ebnf_rules) {
    size_t num_rules = vector_size(ebnf_rules->rules);
    size_t minimum_lookahead = 0UL;
    for(size_t i = 0; i < num_rules; ++i) {
        _ebnf_rule_t rule = vector_get(ebnf_rules->rules, i);
        for(size_t j = i + 1; j < num_rules; ++j) {
            minimum_lookahead = max(minimum_lookahead, 
                _ebnf_rule_index_of_left_most_difference(rule, 
                    vector_get(ebnf_rules->rules, j)));
        }
    }
    return minimum_lookahead;
}

static size_t _ebnf_rule_index_of_left_most_difference(_ebnf_rule_t rule1, _ebnf_rule_t rule2) {
    if(!_terminal_equals(rule1.lhs_terminal, rule2.lhs_terminal))
        return 0;
    
    size_t min_size = min(vector_size(rule1.rule), vector_size(rule2.rule));
    for(size_t i = 0; i < min_size; ++i)
        if(!_terminal_equals(vector_get(rule1.rule, i), vector_get(rule2.rule, i)))
            return i;

    return (vector_size(rule1.rule) == vector_size(rule2.rule)) 
         ? 0UL - 1 
         : min_size;
}

static size_t _terminal_equals(_terminal_t terminal1, _terminal_t terminal2) {
    if(terminal1.is_terminal != terminal2.is_terminal)
        return 0;

    size_t min_size = min(terminal1.name_length, terminal2.name_length);
    for(size_t i = 0; i < min_size; ++i)
        if(terminal1.name[i] != terminal2.name[i])
            return 0;
    
    return terminal1.name_length == terminal2.name_length;
}

#define term_map_get(terminal_vector, index)    _terminal_tree_get_with_default_null(terminal_vector, index)
_terminal_t _terminal_tree_get_with_default_null(Vector(_terminal_t) *terminal_vector, size_t index);

_terminal_tree_t *_ebnf_rules_construct_terminal_tree(_ebnf_rules_t *ebnf_rules, size_t minimum_lookahead) {
    size_t num_rules = vector_size(ebnf_rules->rules);
    _terminal_tree_t *tree = map_new(_terminal_t, _void_ptr_);
    for(size_t i = 0; i < num_rules; ++i) {
        _ebnf_rule_t ebnf_rule = vector_get(ebnf_rules->rules, i);
        _terminal_tree_t *tree_ptr = tree;
        for(size_t j = 0; j < minimum_lookahead; ++j) {
            if(0 == map_count(tree_ptr, term_map_get(ebnf_rule.rule, j))) {
                map_insert(tree_ptr, term_map_get(ebnf_rule.rule, j), 
                    map_new(_terminal_t, _void_ptr_));
                tree_ptr = (_terminal_tree_t*) map_at(tree_ptr, term_map_get(ebnf_rule.rule, j));
            } else {
                tree_ptr = (_terminal_tree_t*) map_at(tree_ptr, term_map_get(ebnf_rule.rule, j));
            }
        }
        map_insert(tree_ptr, term_map_get(ebnf_rule.rule, minimum_lookahead), 
            (void*) i); // potential bug, but we know that the # of rules won't reach the max int limit
    }
}

_terminal_t _terminal_tree_get_with_default_null(Vector(_terminal_t) *terminal_vector, size_t index) {
    return index < vector_size(terminal_vector) 
        ? vector_get(terminal_vector, index) 
        : null_terminal;
}

_ebnf_rules_fn_t _ebnf_rules_fn_impl = {
    &_ebnf_rules_new,
    &_ebnf_rules_add_rule,
    &_ebnf_construct_parse_tree
};

#undef min
#undef max

#endif