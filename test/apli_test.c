#include "../src/apli.h"

#define apli_return_type int

/**
 * --- Arithmetic Rules ---
 * <expr>   := <term>
 * <expr>   := <expr> ('+' | '-') <term>
 * <term>   := <factor>
 * <term>   := <term> ('*' | '/') <factor>
 * <factor> := NUMBER | '(' <expression> ')'
 */

apli_init();
apli_define_functions(expr, term, factor);

__APLI_START__
    apli_non_terminals(expr, term, factor);
    apli_terminals(NUMBER, PLUS, MINUS, STAR, FORWARD_SLASH, OPEN_PAREN, CLOSE_PAREN);

    apli_regex(
        (NUMBER, "(-[123456789][0123456789]*)"),
        (NUMBER, "(\\+?[123456789][0123456789]*)"),
        (NUMBER, "0"),
        (PLUS, "\\+"),
        (MINUS, "-"),
        (STAR, "\\*"),
        (FORWARD_SLASH, "/"),
        (OPEN_PAREN, "\\("),
        (CLOSE_PAREN, "\\)")
    );
    apli_regex_compile();

    apli_bnf_rule(expr, term);
    apli_bnf_rule(expr, expr, PLUS, term);
    apli_bnf_rule(expr, expr, MINUS, term);

    apli_bnf_rule(term, factor);
    apli_bnf_rule(term, term, STAR, factor);
    apli_bnf_rule(term, term, FORWARD_SLASH, factor);

    apli_bnf_rule(factor, NUMBER);
    apli_bnf_rule(factor, OPEN_PAREN, expr, CLOSE_PAREN);

    // parse_tree_result = apli_get_parse_tree("(1 + 1) / 1 * 5 + 3*2 + 5 + 4 / 2");

    // printf("%i\n", apli_evaluate_node(parse_tree_result.root));
    printf("%i\n", apli_evaluate("(1 + 1) / 1 / 5 + 3*2 + 5 + 4 / 2"));
    printf("%i\n", apli_evaluate("(1 + 1) / 1 * 5 + 3*2 + 5 + 4 / 2"));



__APLI_END__

apli_function(expr) {
    size_t sz = vector_size(node.children);
    if(1 == sz)
        return apli_evaluate_node(vector_get(node.children, 0));
    else if(3 == sz) {
        if(0 == strcmp("PLUS", vector_get(node.children, 1).root.ptr.token.name))
            return apli_evaluate_node(vector_get(node.children, 0)) + apli_evaluate_node(vector_get(node.children, 2));
        else
            return apli_evaluate_node(vector_get(node.children, 0)) - apli_evaluate_node(vector_get(node.children, 2));
    }
    assert(0 == "Not reachable.");
    return -1;
}

apli_function(term) {
    size_t sz = vector_size(node.children);
    if(1 == sz) {
        return apli_evaluate_node(vector_get(node.children, 0));
    } else if(3 == sz) {
        if(0 == strcmp("STAR", vector_get(node.children, 1).root.ptr.token.name))
            return apli_evaluate_node(vector_get(node.children, 0)) * apli_evaluate_node(vector_get(node.children, 2));
        else
            return apli_evaluate_node(vector_get(node.children, 0)) / apli_evaluate_node(vector_get(node.children, 2));
    }
    assert(0 == "Not reachable.");
    return -1;
}

apli_function(factor) {
    size_t sz = vector_size(node.children);
    if(1 == sz) {
        _token_t number = vector_get(node.children, 0).root.ptr.token;
        char buf[20];
        // printf("size: %zu\n", number.length);
        for(size_t i = 0; i < number.length; ++i)
            buf[i] = number.ptr[i];
        buf[number.length] = '\0';
        // printf("NUMBER: `%s` -> %i\n", buf, atoi(buf));
        return atoi(buf);
    } else if(3 == sz)
        return apli_evaluate_node(vector_get(node.children, 1));
    assert(0 == "Not reachable.");
    return -1;
}