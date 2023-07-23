#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"

/**
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
 */

#define EbnfRules                                   _ebnf_rules_t
#define terminal_from()
#define ebnf_rule_from()
#define min(x,y)                                    (((x) < (y)) ? (x) : (y))

struct _terminal_ {
    char isNonTerminal : 1;
    const char *name;
    size_t name_length;
};
typedef struct _terminal_ _terminal_t; 

define_vector(_terminal_t);

struct _ebnf_rule_ {
    _terminal_t lhs_terminal;
    Vector(_terminal_t) *rule;
};
typedef struct _ebnf_rule_ _ebnf_rule_t;

define_vector(_ebnf_rule_t);

enum _ebnf_rules_state {EBNF_START, EBNF_COMPILED};
struct _ebnf_rules_ {
    Vector(_ebnf_rule_t) *rules;
};
typedef struct _ebnf_rules_ _ebnf_rules_t;

struct _ebnf_rules_fn_ {
    void (*_add_rule)(_ebnf_rules_t*, _ebnf_rule_t);
    size_t (*_find_minimum_lookahead)(_ebnf_rules_t*);

};
typedef struct _ebnf_rules_fn_ _ebnf_rules_fn_t;

extern _ebnf_rules_fn_t _ebnf_rules_fn_impl;
static size_t _ebnf_rule_index_of_left_most_difference(_ebnf_rule_t rule1, _ebnf_rule_t rule2);
static size_t _terminal_equals(_terminal_t terminal1, _terminal_t terminal2);

void _ebnf_rules_add_rule(_ebnf_rules_t *ebnf_rules, _ebnf_rule_t rule) {
    vector_push_back(ebnf_rules->rules, rule);
}

size_t _ebnf_rules_find_minimum_lookahead(_ebnf_rules_t *ebnf_rules) {
    size_t num_rules = vector_size(ebnf_rules->rules);
    size_t minimum_lookahead = 0UL - 1;
    for(size_t i = 0; i < num_rules; ++i) {
        // for each rule, 
        _ebnf_rule_t rule = vector_get(ebnf_rules->rules, i);
        for(size_t j = i + 1; j < num_rules; ++j) {
            minimum_lookahead = min(minimum_lookahead, 
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
    for(size_t i = 0; i < min_size; ++i) {
        if(!_terminal_equals(vector_get(rule1.rule, i), vector_get(rule2.rule, i)))
            return i;
    }
    return (vector_size(rule1.rule) == vector_size(rule2.rule)) 
         ? 0UL - 1 
         : min_size;
}

static size_t _terminal_equals(_terminal_t terminal1, _terminal_t terminal2) {
    if(terminal1.isNonTerminal != terminal2.isNonTerminal)
        return 0;
    size_t min_size = min(terminal1.name_length, terminal2.name_length);
    for(size_t i = 0; i < min_size; ++i) {
        if(terminal1.name[i] != terminal2.name[i])
            return 0;
    }
    return terminal1.name_length == terminal2.name_length;
}

_ebnf_rules_fn_t _ebnf_rules_fn_impl = {
    &_ebnf_rules_add_rule,
    &_ebnf_rules_find_minimum_lookahead
};

#endif