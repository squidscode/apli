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

#define Terminal                                            _terminal_t
#define terminal_from(name)                                 (_terminal_from(1, (name), strlen((name))))
#define non_terminal_from(name)                             (_terminal_from(0, (name), strlen((name))))
#define terminal_from_segment(name, length)                 (_terminal_from(1, (name), (length)))
#define non_terminal_from_segment(name, length)             (_terminal_from(0, (name), (length)))
#define EbnfRules                                           _ebnf_rules_t
#define ebnf_rules_new()                                    (_ebnf_rules_new())
#define ebnf_rules_add_rule(ebnf_rules, ebnf_rule)          (_ebnf_rules_fn_impl._add_rule((ebnf_rules), (ebnf_rule)))
#define ebnf_rules_construct_parse_tree(ebnf_rules, tokens) (_ebnf_rules_fn_impl._construct_parse_tree((ebnf_rules), (tokens)))
#define ebnf_rule_from(lhs, ...)                            (_ebnf_rule_from((lhs), PP_NARG(__VA_ARGS__), __VA_ARGS__))
#define ebnf_rule_from_vector(lhs, rule_vec)                (_ebnf_rule_from_vec((lhs), (rule_vec)))
#define min(x,y)                                            (((x) < (y)) ? (x) : (y))
#define max(x,y)                                            (((x) < (y)) ? (y) : (x))

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

// TODO Remove name_length from here so that it is easier to write the evaluator.

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
    _terminal_t terminal;
    _token_t token;
};
typedef union _terminal_or_token_ptr_ _terminal_or_token_ptr_t;

struct _parse_tree_value_ {
    char is_terminal_t : 1;
    _terminal_or_token_ptr_t ptr;
};
typedef struct _parse_tree_value_ _parse_tree_value_t;

typedef struct __parse_tree_node_t_vector_ __parse_tree_node_t_vector_t;
struct _parse_tree_node_ {
    _parse_tree_value_t root;
    Vector(_parse_tree_node_t) *children;
};
typedef struct _parse_tree_node_ _parse_tree_node_t;
define_vector(_parse_tree_node_t);

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
void _print_terminal(_terminal_t term);
_parse_tree_t _ebnf_rules_shift_reduce_parse(_ebnf_rules_t*, List(_token_t)*, _terminal_tree_t*, size_t);

EbnfRules* _ebnf_rules_new() {
    EbnfRules *ebnf_rules = (EbnfRules*) malloc(sizeof(EbnfRules));
    ebnf_rules->rules = vector_new(_ebnf_rule_t);
    return ebnf_rules;
}

void _ebnf_rules_add_rule(_ebnf_rules_t *ebnf_rules, _ebnf_rule_t rule) {
    if(1 == rule.lhs_terminal.is_terminal)
        assert("LHS terminal cannot be a terminal");
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

// TODO delete / mark the following function bc it is only for debugging
#define print_ebnf_rules_terminal_tree(tt, lh)      _print_ebnf_rules_terminal_tree(tt, lh, 0)
void _print_ebnf_rules_terminal_tree(_terminal_tree_t *, size_t, size_t);

_parse_tree_t _ebnf_construct_parse_tree(_ebnf_rules_t *rules, List(_token_t) *token_list) {
    size_t minimum_lookahead = _ebnf_rules_find_minimum_lookahead(rules);
    _terminal_tree_t *terminal_tree = _ebnf_rules_construct_terminal_tree(rules, minimum_lookahead);
    // begin shift-reduce with terminal_tree:
    // print_ebnf_rules_terminal_tree(terminal_tree, minimum_lookahead);
    return _ebnf_rules_shift_reduce_parse(rules, token_list, terminal_tree, minimum_lookahead);
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
static inline size_t _terminal_tree_key_hash(_terminal_t);
static inline size_t _terminal_tree_key_equals(_terminal_t, _terminal_t);

_terminal_tree_t *_ebnf_rules_construct_terminal_tree(_ebnf_rules_t *ebnf_rules, size_t num_terms) {
    size_t num_rules = vector_size(ebnf_rules->rules);
    _terminal_tree_t *tree = map_new(_terminal_t, _void_ptr_);
    map_set_hash(tree, &_terminal_tree_key_hash);
    map_set_key_eq(tree, &_terminal_tree_key_equals);
    for(size_t i = 0; i < num_rules; ++i) {
        _ebnf_rule_t ebnf_rule = vector_get(ebnf_rules->rules, i);
        _terminal_tree_t *tree_ptr = tree;
        for(size_t j = 0; j < num_terms; ++j) {
            // printf("Adding terminal: "); _print_terminal(term_map_get(ebnf_rule.rule, j));
            // printf("\n");
            if(0 == map_count(tree_ptr, term_map_get(ebnf_rule.rule, j))) {
                map_insert(tree_ptr, term_map_get(ebnf_rule.rule, j), 
                    map_new(_terminal_t, _void_ptr_));
                tree_ptr = (_terminal_tree_t*) map_at(tree_ptr, term_map_get(ebnf_rule.rule, j));
            } else {
                tree_ptr = (_terminal_tree_t*) map_at(tree_ptr, term_map_get(ebnf_rule.rule, j));
            }
            map_set_hash(tree_ptr, &_terminal_tree_key_hash);
            map_set_key_eq(tree_ptr, &_terminal_tree_key_equals);
        }
        // printf("Adding terminal: "); _print_terminal(term_map_get(ebnf_rule.rule, minimum_lookahead));
        // printf("\n");
        map_insert(tree_ptr, term_map_get(ebnf_rule.rule, num_terms), 
            (void*) i); // potential bug, but we know that the # of rules won't reach the max int / long limit
        // printf("\n\n");
    }
    
    return tree;
}

static inline size_t _terminal_tree_key_hash(_terminal_t terminal) {
    size_t hash = 0;
    char *ptr = (char*) ((void*) &terminal.name);
    size_t i = 0;
    size_t mod = sizeof(size_t) / sizeof(char);
    size_t offset = 0;
    while(i < terminal.name_length) {
        hash ^= ((255UL & ptr[i++]) << (8 * offset++));
        offset %= mod;
    }
    // printf("<<hash for `");
    // for(size_t i = 0; i < terminal.name_length; ++i)
    //     printf("%c", terminal.name[i]);
    // printf("` is ");
    // printf("%zu>>", hash);
    return hash;
}

static inline size_t _terminal_tree_key_equals(_terminal_t terminal_1, _terminal_t terminal_2) {
    if(terminal_1.name_length != terminal_2.name_length) {
        // printf("<<invalid name length!>>"); 
        return 0;
    }
    for(size_t i = 0; i < terminal_1.name_length; ++i)
        if(terminal_1.name[i] != terminal_2.name[i]) {
            // printf("<<incorrect char @ %zu!>>", i); 
            return 0;
        }
    return 1;
}

_terminal_t _terminal_tree_get_with_default_null(Vector(_terminal_t) *terminal_vector, size_t index) {
    return index < vector_size(terminal_vector) 
        ? vector_get(terminal_vector, index) 
        : null_terminal;
}

void _print_ebnf_rules_terminal_tree(_terminal_tree_t *terminal_tree, size_t minimum_lookahead, size_t tab_size) {
    __terminal_t__void_ptr__map_match_t_list_t *list = map_get_list(terminal_tree);
    while(list_size(list)) {
        __terminal_t__void_ptr__map_match_t nxt = list_get_front(list);
        for(size_t i = 0; i < tab_size; ++i)
            printf("    ");
        if (minimum_lookahead == 0) {
            _print_terminal(nxt.key);
            printf(" -> RULE #%zu\n", (size_t) nxt.value);
        } else {
            _print_terminal(nxt.key);
            printf("\n");
            _print_ebnf_rules_terminal_tree((_terminal_tree_t*) nxt.value, minimum_lookahead - 1, tab_size+1);
        }
        list_pop_front(list);
    }
}

void _print_terminal(_terminal_t term) {
    if(term.is_terminal)
        printf("[TERM");
    else
        printf("[NON-TERM");
    printf(" \"");
    for(size_t i = 0; i < term.name_length; ++i) {
        printf("%c", term.name[i]);
    }
    printf("\"]");
}

static inline _parse_tree_node_t _parse_tree_node_t_from_token_t(_token_t token);
static inline void _parser_shift(Vector(_parse_tree_node_t)*, List(_token_t)*);
static inline void _parser_fill_look_ahead_list(List(_token_t)*, List(_token_t)*, size_t look_ahead);
static inline char _parser_shift_condition(Vector(_parse_tree_node_t)*, List(_token_t)*, _terminal_tree_t*, size_t);
static inline char _parser_reduce(Vector(_parse_tree_node_t)*, _ebnf_rules_t*);
static inline void _parser_print_parse_tree_node_vector(Vector(_parse_tree_node_t)*);
static inline void _parser_print_token_list(List(_token_t)*);
static inline void _parser_print_parsing_step(Vector(_parse_tree_node_t) *parse_stack, List(_token_t) *look_ahead_list,
    List(_token_t) *token_list, size_t step_number);

_parse_tree_t _ebnf_rules_shift_reduce_parse(_ebnf_rules_t *rules, List(_token_t) *token_list, _terminal_tree_t *tree, size_t look_ahead) {
    Vector(_parse_tree_node_t) *parse_stack = vector_new(_parse_tree_node_t);
    List(_token_t) *look_ahead_list = list_new(_token_t);
    size_t step_number = 1;

    if(0 == list_size(token_list))
        assert(0 == "Parser error!");

    // To begin, we fill the look_ahead list, shift, then fill look_ahead again.
    _parser_fill_look_ahead_list(look_ahead_list, token_list, look_ahead);
    _parser_shift(parse_stack, look_ahead_list);
    // _parser_print_parsing_step(parse_stack, look_ahead_list, token_list, step_number);
    step_number += 1;
    _parser_fill_look_ahead_list(look_ahead_list, token_list, look_ahead);

    while(0 < list_size(token_list) || 0 < list_size(look_ahead_list)) {
        // _parser_print_parsing_step(parse_stack, look_ahead_list, token_list, step_number);
        step_number += 1;
        if(_parser_shift_condition(parse_stack, look_ahead_list, tree, look_ahead)) {
            _parser_shift(parse_stack, look_ahead_list);
            _parser_fill_look_ahead_list(look_ahead_list, token_list, look_ahead);
        } else {
            if(1 == _parser_reduce(parse_stack, rules)) {
                _parser_shift(parse_stack, look_ahead_list);
                _parser_fill_look_ahead_list(look_ahead_list, token_list, look_ahead);
            }
        }
    }

    // Keep reducing.
    while(0 == _parser_reduce(parse_stack, rules)) {
        // _parser_print_parsing_step(parse_stack, look_ahead_list, token_list, step_number++);
        step_number += 1;
    }

    // NOTE: Uncomment to display the parse_tree. 
    // _parser_print_parsing_step(parse_stack, look_ahead_list, token_list, step_number);

    list_free(look_ahead_list);
    if(1 != vector_size(parse_stack))
        assert(0 == "Parser error!");
    _parse_tree_t parse_tree = {vector_get_back(parse_stack)};
    vector_free(parse_stack);
    return parse_tree;
}

#define FGREEN  "\x1b[32;1m"
#define FYELLOW  "\x1b[33;1m"
#define FBLUE  "\x1b[34;1m"
#define FMAGENTA  "\x1b[35;1m"
#define FCYAN  "\x1b[36;1m"
#define FLBLUE  "\x1b[38;2;50;175;255;1m"
#define FRED  "\x1b[31;1m"
#define RESET "\x1b[0m"

static inline void _parser_print_parsing_step(Vector(_parse_tree_node_t) *parse_stack, List(_token_t) *look_ahead_list,
    List(_token_t) *token_list, size_t step_number) {
    printf(FBLUE "----------- STEP #%zu -----------" RESET, step_number);
    printf("\n\n" FRED "Parse Stack: " RESET "\n");
    _parser_print_parse_tree_node_vector(parse_stack);
    printf(FRED "Look-ahead list: " RESET);
    _parser_print_token_list(look_ahead_list);
    printf("\n" FRED "Tokens: " RESET);
    _parser_print_token_list(token_list);
    printf("\n\n");
    printf(FBLUE "---------------------------------\n\n" RESET);
}


static inline void _parser_shift(Vector(_parse_tree_node_t) *parse_stack, List(_token_t) *look_ahead_list) {
    if(0 < list_size(look_ahead_list)) {
        vector_push_back(parse_stack, _parse_tree_node_t_from_token_t(list_get_front(look_ahead_list)));
        list_pop_front(look_ahead_list);
    } else {
        assert(0 == "Cannot shift from an empty look_ahead_list");
    }
}

static inline void _parser_fill_look_ahead_list(List(_token_t) *look_ahead_list, List(_token_t) *token_list, 
    size_t look_ahead) {
    while(list_size(look_ahead_list) < max(look_ahead - 1, 1) && 0 < list_size(token_list)) {
        list_push_back(look_ahead_list, list_get_front(token_list));
        list_pop_front(token_list);
    }
}

static inline _parse_tree_node_t _parse_tree_node_t_from_token_t(_token_t token) {
    _parse_tree_value_t ptv;
    ptv.is_terminal_t = 0;
    ptv.ptr.token = token;
    _parse_tree_node_t ptn = {ptv, vector_new(_parse_tree_node_t)};
    return ptn;
}

// We shift iff the [ top(parse_stack), look_ahead_list ] hits a rule in a in the terminal tree.
static inline char _parser_shift_condition(Vector(_parse_tree_node_t) *parse_stack, List(_token_t) *look_ahead_list,
    _terminal_tree_t *tree, size_t look_ahead) {
    _terminal_tree_t *tree_ptr = tree;

    _terminal_t terminal = vector_get_back(parse_stack).root.is_terminal_t 
      ? vector_get_back(parse_stack).root.ptr.terminal
      : terminal_from(vector_get_back(parse_stack).root.ptr.token.name);

    // printf("[[`%s` is ", terminal.name);
    // printf(map_count(tree_ptr, terminal) ? "" : "NOT ");
    // printf("in the map]]\n");

    if(map_count(tree_ptr, terminal))
        tree_ptr = (_terminal_tree_t*) map_at(tree_ptr, terminal);
    else
        return 0;

    Iterator(_token_t) *look_ahead_iter = list_get_iterator(look_ahead_list);
    while(iter_is_not_null(look_ahead_iter)) {
        terminal = non_terminal_from(iter_val(look_ahead_iter).name);
        // printf("[[`%s` is ", iter_val(look_ahead_iter).name);
        // printf(map_count(tree_ptr, terminal) ? "" : "NOT ");
        // printf("in the map]]\n");
        if(map_count(tree_ptr, terminal))
            tree_ptr = (_terminal_tree_t*) map_at(tree_ptr, terminal);
        else
            return 0;
        look_ahead_iter = iter_next(look_ahead_iter);
    }

    // printf("Shift recommended! Rule #%zu can be enacted.\n", (size_t) tree_ptr);
    return 1;
}

static inline char _parser_parse_stack_matches_ebnf_rule(Vector(_parse_tree_node_t) *parse_stack, _ebnf_rule_t ebnf);

define_vector(size_t);
static inline char _parser_reduce(Vector(_parse_tree_node_t) *parse_stack, _ebnf_rules_t *ebnf_rules) {
    Vector(size_t) *possible_rule_indices = vector_new(size_t);
    for(size_t i = 0; i < vector_size(ebnf_rules->rules); ++i) {
        vector_push_back(possible_rule_indices, i);
    }

    // Sort from largest stack size to lowest stack size (bubble sort implementation)
    for(size_t i = 0; i < vector_size(possible_rule_indices); ++i) {
        for(size_t j = 0; j < vector_size(possible_rule_indices) - 1; ++j) {
            if(vector_size(vector_get(ebnf_rules->rules, vector_get(possible_rule_indices, j)).rule)
              < vector_size(vector_get(ebnf_rules->rules, vector_get(possible_rule_indices, j + 1)).rule)) {
                size_t tmp = vector_get(possible_rule_indices, j);
                vector_set(possible_rule_indices, j, vector_get(possible_rule_indices, j + 1));
                vector_set(possible_rule_indices, j + 1, tmp);
            }
        }
    }

    for(size_t i = 0; i < vector_size(possible_rule_indices); ++i) {
        _ebnf_rule_t ebnf = vector_get(ebnf_rules->rules, vector_get(possible_rule_indices, i));
        // printf("Checking Rule #%zu!\n", vector_get(possible_rule_indices, i));
        if(_parser_parse_stack_matches_ebnf_rule(parse_stack, ebnf)) {
            // printf("Rule #%zu matched!\n", vector_get(possible_rule_indices, i));
            Vector(_parse_tree_node_t) *children_vector = vector_new(_parse_tree_node_t);
            size_t parse_stack_size = vector_size(parse_stack);
            for(size_t j = parse_stack_size - vector_size(ebnf.rule); j < parse_stack_size; ++j)
                vector_push_back(children_vector, vector_get(parse_stack, j));
            for(size_t j = 0; j < vector_size(ebnf.rule); ++j)
                vector_pop_back(parse_stack);
            _parse_tree_value_t parent_value = {-1, ebnf.lhs_terminal};
            _parse_tree_node_t new_parent_node = {parent_value, children_vector};
            vector_push_back(parse_stack, new_parent_node);
            return 0; // successfully reduced!
        }
   }

    vector_free(possible_rule_indices);
    return 1; // Could not reduce!
}

static inline char _parser_parse_stack_matches_ebnf_rule(Vector(_parse_tree_node_t) *parse_stack, _ebnf_rule_t ebnf) {
    // printf("parse tree stack size : %zu\n", vector_size(parse_stack));
    // printf("vector_size(ebnf.rule) = %zu\n", vector_size(ebnf.rule));
    for(size_t i = vector_size(ebnf.rule) - 1; i != 0UL - 1; --i) {
        // printf("i = %zu\n", i);
        size_t parse_stack_index = vector_size(parse_stack) - ((vector_size(ebnf.rule) - 1) - i) - 1;
        // printf("parse_stack_index = %zu\n", parse_stack_index);
        if(vector_size(parse_stack) <= parse_stack_index)
            return 0;
        // printf("checking index: %zu\n", parse_stack_index);
        _parse_tree_value_t ptv = vector_get(parse_stack, parse_stack_index).root;
        const char *ptv_name = ptv.is_terminal_t ? ptv.ptr.terminal.name : ptv.ptr.token.name;
        size_t ptv_name_length = ptv.is_terminal_t ? ptv.ptr.terminal.name_length : strlen(ptv.ptr.token.name);
        if(ptv_name_length != vector_get(ebnf.rule, i).name_length
            || 0 != memcmp(vector_get(ebnf.rule, i).name, ptv_name, ptv_name_length))
            return 0;
    }
    return 1;
}

static inline void _parser_print_parse_tree_node_vector_helper(Vector(_parse_tree_node_t)*, size_t);
static inline void _parser_print_token(_token_t);

static inline void _parser_print_parse_tree_node_vector(Vector(_parse_tree_node_t) *tree) {
    _parser_print_parse_tree_node_vector_helper(tree, 0);
}

static inline void _parser_print_parse_tree_value(_parse_tree_value_t value);

static inline void _parser_print_parse_tree_node_vector_helper(Vector(_parse_tree_node_t) *tree, size_t indent) {
    size_t size = vector_size(tree);
    for(size_t i = 0; i < size; ++i) {
        for(size_t indent_level = 0; indent_level < indent; ++indent_level)
            printf("|   ");
        _parse_tree_node_t node = vector_get(tree, i);
        _parser_print_parse_tree_value(node.root);
        printf("\n");
        _parser_print_parse_tree_node_vector_helper(node.children, indent + 1);
    }
}

static inline void _parser_print_parse_tree_value(_parse_tree_value_t value) {
    printf("<");
    if(value.is_terminal_t) {
        // printf(value.ptr.terminal.is_terminal ? "TERM" : "NON_TERM");
        // printf(" ");
        for(size_t i = 0; i < value.ptr.terminal.name_length; ++i) {
            printf("%c", value.ptr.terminal.name[i]);
        }
    } else {
        _parser_print_token(value.ptr.token);
    }
    printf(">");
}

static inline void _parser_print_token_list(List(_token_t) *tokens) {
    Iterator(_token_t) *iter = list_get_iterator(tokens);
    printf("(");
    char first = 1;
    while(iter_is_not_null(iter)) {
        if(!first) printf(", ");
        first = 0;
        _parser_print_token(iter_val(iter));
        iter = iter_next(iter);
    }
    printf(")");
}

static inline void _parser_print_token(_token_t token) {
    printf("`%s` ", token.name);
    for(size_t i = 0; i < token.length; ++i) 
        printf("%c", token.ptr[i]);
}

_ebnf_rules_fn_t _ebnf_rules_fn_impl = {
    &_ebnf_rules_new,
    &_ebnf_rules_add_rule,
    &_ebnf_construct_parse_tree
};

#undef min
#undef max

#endif