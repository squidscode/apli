#include "../../src/apli.h"

#define APLI_EVAL_ARGUMENTS
#define APLI_EVAL_NAMES
#define APLI_EVAL_RETURN_TYPE int

/**
 * --- Arithmetic Rules ---
 * <expr>   := <term>
 * <expr>   := <expr> ('+' | '-') <term>
 * <term>   := <factor>
 * <term>   := <term> ('*' | '/') <factor>
 * <factor> := NUMBER | '(' <expression> ')' | <factor> '^' NUMBER
 */

apli_init();
apli_define_functions(expr, term, factor);

char *add_pre_post_buffer(const char *str, size_t buffer_size);

__APLI_START__

    if(2 != argc)
        assert(0 == "The second argument must be the arithmetic expression");

    apli_non_terminals(expr, term, factor);
    apli_terminals(NUMBER, PLUS, MINUS, STAR, FORWARD_SLASH, OPEN_PAREN, CLOSE_PAREN, CARROT);

    apli_regex(
        (NUMBER, "(\\-?([123456789][0123456789]*|0))"),
        (PLUS, "\\+"),
        (MINUS, "-"),
        (STAR, "\\*"),
        (CARROT, "\\^"), // CARROT?
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
    apli_bnf_rule(factor, factor, CARROT, NUMBER);
    apli_bnf_rule(factor, OPEN_PAREN, expr, CLOSE_PAREN);

    char *input = add_pre_post_buffer(argv[1], 10);
    printf("%i\n", apli_evaluate(input));

    free(input);

__APLI_END__

apli_function(expr) {
    size_t sz = apli_num_children();
    if(1 == sz)
        return apli_eval_child(1);
    else if(3 == sz) {
        if(apli_child_token_name_equals(PLUS, 2))
            return apli_eval_child(1) + apli_eval_child(3);
        else
            return apli_eval_child(1) - apli_eval_child(3);
    }
    assert(0 == "Not reachable.");
    return -1;
}

apli_function(term) {
    size_t sz = apli_num_children();
    if(1 == sz) {
        return apli_eval_child(1);
    } else if(3 == sz) {
        if(apli_child_token_name_equals(STAR, 2))
            return apli_eval_child(1) * apli_eval_child(3);
        else
            return apli_eval_child(1) / apli_eval_child(3);
    }
    assert(0 == "Not reachable.");
    return -1;
}


static inline int NUM_to_int(ApliToken number);

apli_function(factor) {
    size_t sz = apli_num_children();
    if(1 == sz) {
        // Pull the number out of the token and load it into a buffer:
        return NUM_to_int(apli_get_child_token(1));
    } else if(3 == sz) {
        if(apli_child_token_name_equals(NUMBER, 3)) {
            int ret = 1;
            int base = apli_eval_child(1);
            int pow = NUM_to_int(apli_get_child_token(3));
            for(int times = 0; times < pow; ++times)
                ret *= base;
            return ret;
        } else {
            return apli_eval_child(2);
        }
        
    }
    assert(0 == "Not reachable.");
    return -1;
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

static inline int NUM_to_int(ApliToken number) {
    char buf[20];
    for(size_t i = 0; i < apli_token_reflen(number); ++i)
        buf[i] = apli_token_ref(number)[i];
    buf[number.length] = '\0';
    return atoi(buf);
}