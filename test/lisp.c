#include "../src/apli.h"

#define apli_return_type int

/**
 * Reference: 
 *   - https://github.com/antlr/grammars-v4/blob/master/lisp/lisp.g4
 *   - https://iamwilhelm.github.io/bnf-examples/lisp
 *
 *
 * --- Lisp Ebnf Rules ---
 * s_expression = atomic_symbol
 *              | "(" s_expression "." s_expression ")" \
 *              | list
 * list = "(" s_expression+ ")"
 * atomic_symbol = letter atom_part
 * atom_part = empty | letter atom_part | number atom_part
 * letter = "a" | "b" | " ..." | "z"
 * number = "1" | "2" | " ..." | "9"
 * empty = " "
 *
 *
 *
 * -- Simplified Lisp Bnf using Regex ---
 * s_expression  := atomic_symbol
 * s_expression  := "(" s_expression "." s_expressison ")"
 * s_expression  := list
 * list          := "(" s_expressions ")"
 * s_expressions := s_expression s_expressions       ** NOTICE: s_expressions == s_expression+
 * s_expressions := s_expression
 * atomic_symbol := r"[^a-z1-9][a-z1-9]+[^a-z1-9]" [OFFSET: +1 ; -1]
 */

apli_init();
apli_define_functions(s_expression, list, s_expressions, atomic_symbol);

char *add_pre_post_buffer(const char *str, size_t buffer_size);

__APLI_START__
    // If we didn't parse right to left, then the parser errors on "(A B C)". This is due to the
    // lack of a `s_expressions := s_expressions s_expressions` rule. See the README for more information.
    // I'd suggest you try to reason about it yourself! Use the `-DPRINT_PARSE_TREE` or `-DPRINT_PARSE_TREE_STEPS`
    // compiler flags and comment out the line below to use the default `LEFT_TO_RIGHT` parser.
    apli_set_parser_type(RIGHT_TO_LEFT);

    if(2 != argc)
        assert(0 == "The second argument must be the arithmetic expression");

    apli_non_terminals(s_expression, list, s_expressions, atomic_symbol);
    apli_terminals(ATOMIC_SYMBOL, OPEN_PAREN, CLOSE_PAREN, PERIOD);

    apli_regex(
        (ATOMIC_SYMBOL, "[^a-z1-9][a-z1-9]+[^a-z1-9]", 1, 1),
        (OPEN_PAREN, "\\("),
        (CLOSE_PAREN, "\\)"),
        (PERIOD, ".")
    );
    apli_regex_compile();

    apli_bnf_rule(s_expression, atomic_symbol);
    apli_bnf_rule(s_expression, OPEN_PAREN, s_expression, PERIOD, s_expression, CLOSE_PAREN);
    apli_bnf_rule(s_expression, list);
    apli_bnf_rule(list, OPEN_PAREN, s_expressions, CLOSE_PAREN);
    apli_bnf_rule(s_expressions, s_expression);
    apli_bnf_rule(s_expressions, s_expression, s_expressions);
    // apli_bnf_rule(s_expressions, s_expressions, s_expressions); // this isn't needed because we go right->left
    apli_bnf_rule(atomic_symbol, ATOMIC_SYMBOL);

    char *input = add_pre_post_buffer(argv[1], 10);
    apli_evaluate(input);

    apli_evaluate(input);

    free(input);

__APLI_END__

apli_function(s_expressions) {
    
}

apli_function(s_expression) {

}

apli_function(atomic_symbol) {

}

apli_function(list) {

}

char *add_pre_post_buffer(const char *str, size_t buffer_size) {
    char *cpy = (char*) malloc(strlen(str) + 2 * buffer_size + 1);
    const size_t sz = strlen(str);
    for(size_t i = 0; i < buffer_size; ++i) {
        cpy[i] = ' ';
    }
    for(size_t i = buffer_size; i < buffer_size + sz; ++i) {
        cpy[i] = str[i - buffer_size];
    }
    for(size_t i = buffer_size + sz; i < 2 * buffer_size + sz; ++i) {
        cpy[i] = ' ';
    }
    cpy[2 * buffer_size + sz] = '\0';
    return cpy;
}