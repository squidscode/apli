#include "../src/apli.h"

#define APLI_EVAL_ARGUMENTS     environment *env
#define APLI_EVAL_NAMES         env
#define APLI_EVAL_RETURN_TYPE   return_value

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

typedef struct _string_segment {
    const char *str;
    size_t length;
} string_segment;

typedef string_segment identifier;
typedef enum _rv_type {NUMBER, IDENTIFIER} rv_type;
/**
 * NUMBER -> ref is size_t
 * IDENTIFIER -> ref is string_segment
 */
typedef union _rv_data {
    int num;
    string_segment segment;
} rv_data;
typedef struct _return_value {
    rv_type type;
    rv_data ref;
} return_value;


define_map(identifier, return_value);
typedef Map(identifier, return_value)* frame;
define_vector(frame);

typedef struct _environment {
    Vector(frame) *stack_frame;
} environment;

#define resolve_id(env, id)         _resolve_identifier(env, id)
#define push_frame(env)             _push_frame(env)
#define pop_frame(env)              _pop_frame(env)
#define extend_env(env, id, val)    _extend_env(env, id, val)
#define seg_to_str(seg)             _segment_to_str(seg)
#define seg_eq_str(seg, str)            _segment_eq_str(seg, str)
#define str_to_seg(str, len)        _str_to_segment(str, len)

return_value _resolve_identifier(environment *env, string_segment id);
void _push_frame(environment *env);
void _pop_frame(environment *env);
void _extend_env(environment *env, string_segment id, return_value rv);

const char *_segment_to_str(string_segment segment);
string_segment _str_to_segment(const char* str, size_t len);
size_t _segment_eq_str(string_segment segment, const char *str);

apli_init();
apli_define_functions(s_expression, list, s_expressions, atomic_symbol);

char *add_pre_post_buffer(const char *str, size_t buffer_size);
void print_return_value(return_value val);

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
        (ATOMIC_SYMBOL, "[^a-z0-9][a-z0-9]+[^a-z0-9]", 1, 1),
        (ATOMIC_SYMBOL, "(<=|>=)", 0, 0),
        (ATOMIC_SYMBOL, "([+-\\*/<>=])", 0, 0),
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

    environment *env = (environment*) malloc(sizeof(environment));
    env->stack_frame = vector_new(frame);
    push_frame(env);
    print_return_value(apli_evaluate(input));

    pop_frame(env);
    vector_free(env->stack_frame);
    free(env);

    free(input);

__APLI_END__

apli_function(s_expressions) {
    // The first value in an s_expressions is always an s_expression.
    if(1 == apli_num_children()) // s_expressions := s_expression
        return apli_eval_child(1);
    else {
        // s_expressions := s_expression s_expressions
        apli_eval_child(1);
        return apli_eval_child(2);
    }
    assert(0 == "Not reachable");
}

apli_function(s_expression) {
    if(apli_child_terminal_name_equals(atomic_symbol, 1)) {
        // s_expression = atomic_symbol
        return apli_eval_child(1);
    } else if(apli_child_token_name_equals(OPEN_PAREN, 1)) {
        // s_expression = "(" s_expression "." s_expressison ")"
    } else {
        // s_expression = list
        return apli_eval_child(1);
    }
    assert(0 == "Not reachable");
}

apli_function(atomic_symbol) {
    // atomic_symbol := r"[^a-z1-9][a-z1-9]+[^a-z1-9]" [OFFSET: +1 ; -1]
    ApliToken tok = apli_get_child_token(1);
    string_segment segment = {apli_token_ref(tok), apli_token_reflen(tok)};
    if('0' <= apli_token_ref(tok)[0] && apli_token_ref(tok)[0] <= '9') {
        return_value rv;
        rv.type = NUMBER;
        rv.ref.num = atoi(seg_to_str(segment));
        return rv;
    } else {
        return_value rv;
        rv.type = IDENTIFIER;
        rv.ref.segment = segment;
        return rv;
    }
    
    assert(0 == "Not reachable");
}

return_value lisp_call(return_value id, Vector(_parse_tree_node_t) *children, environment*);

apli_function(list) {
    // list          := "(" s_expressions ")"
    ApliNode sexprs = apli_get_child(2);
    return lisp_call(apli_evaluate_node(vector_get(sexprs.children, 0)), sexprs.children, env);
    assert(0 == "Not reachable");
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

return_value _resolve_identifier(environment *env, string_segment id) {
    Vector(frame) *sf = env->stack_frame;
    size_t sz = vector_size(sf);
    for(size_t i = sz - 1; i < sz; --i) {
        if(map_count(vector_get(sf, i), id)) {
            return map_at(vector_get(sf, i), id);
        }
    }
    printf("Invalid Identifier Error! Identifier `%s` is not bound.\n", seg_to_str(id));
    assert(0 == "Invalid identifier");
}

size_t seg_hash(string_segment seg) {
    size_t hash = 0;
    char *ptr = (char*) ((void*) &seg.str);
    size_t i = 0;
    size_t mod = sizeof(size_t) / sizeof(char);
    size_t offset = 0;
    while(i < seg.length) {
        hash ^= ((255UL & ptr[i++]) << (8 * offset++));
        offset %= mod;
    }
    return hash; 
}

size_t seg_eq(string_segment seg1, string_segment seg2) {
    if(seg1.length != seg2.length)
        return 0;
    for(size_t i = 0; i < seg1.length; ++i)
        if(seg1.str[i] != seg2.str[i])
            return 0;
    return 1;
}

void _push_frame(environment *env) {
    frame f = map_new(identifier, return_value);
    map_set_hash(f, &seg_hash);
    map_set_key_eq(f, &seg_eq);
    vector_push_back(env->stack_frame, f);

}

void _pop_frame(environment *env) {
    map_free(vector_get_back(env->stack_frame));
    assert(0 < vector_size(env->stack_frame));
    vector_pop_back(env->stack_frame);
}

void _extend_env(environment *env, string_segment id, return_value rv) {
    map_insert(vector_get_back(env->stack_frame), id, rv);
}

#define LOOP_OVER_REST_SEXPRS(children, result_type, step_expr) \
    if(1 < vector_size(children)) { \
        ApliNode node = vector_get(children, 1); \
        while(apli_node_terminal_name_equals(node, s_expressions)) { \
            return_value result = apli_evaluate_child(1); \
            if(result_type != result.type) { \
                printf("Argument must be " #result_type "! Result: "); \
                print_return_value(result); \
                assert(0 == "Invalid argument!"); \
            } \
            step_expr; \
            if(vector_size(node.children) < 2) \
                break; \
            node = apli_get_child(2); \
        } \
    }

size_t return_value_is_truthy(return_value);

return_value lisp_call(return_value id, Vector(_parse_tree_node_t) *children, environment *env) {
    if(NUMBER == id.type) {
        printf("Number `%d` is not callable.\n", id.ref.num);
        assert(0 == "Invalid call!");
    } else if(IDENTIFIER == id.type) {
        if(seg_eq_str(id.ref.segment, "+")) {
            int total = 0;
            LOOP_OVER_REST_SEXPRS(children, NUMBER, total += result.ref.num);
            return_value rv;
            rv.type = NUMBER;
            rv.ref.num = total;
            return rv;
        } else if(seg_eq_str(id.ref.segment, "*")) {
            int total = 1;
            LOOP_OVER_REST_SEXPRS(children, NUMBER, total *= result.ref.num);
            return_value rv;
            rv.type = NUMBER;
            rv.ref.num = total;
            return rv;
        } else if(seg_eq_str(id.ref.segment, "-")) {
            ApliNode node = vector_get(children, 1); \
            return_value ret = apli_evaluate_child(1);
            assert(NUMBER == ret.type);
            int total = ret.ref.num;
            assert(1 < vector_size(children));
            children = vector_get(children, 1).children;
            LOOP_OVER_REST_SEXPRS(children, NUMBER, total -= result.ref.num);
            return_value rv;
            rv.type = NUMBER;
            rv.ref.num = total;
            return rv;
        } else if(seg_eq_str(id.ref.segment, "/")) {
            ApliNode node = vector_get(children, 1);
            return_value ret = apli_evaluate_child(1);
            assert(NUMBER == ret.type);
            int total = ret.ref.num;
            assert(1 < vector_size(children));
            children = vector_get(children, 1).children;
            LOOP_OVER_REST_SEXPRS(children, NUMBER, total /= result.ref.num);
            return_value rv;
            rv.type = NUMBER;
            rv.ref.num = total;
            return rv;
        } else if (seg_eq_str(id.ref.segment, "=")) {
            ApliNode node = vector_get(children, 1);
            return_value val1 = apli_evaluate_child(1);
            node = apli_get_child(2);
            return_value val2 = apli_evaluate_child(1);
            return_value ret;
            ret.type = NUMBER;
            ret.ref.num = NUMBER == val1.type && NUMBER == val2.type && val1.ref.num == val2.ref.num;
            return ret;
        } else if (seg_eq_str(id.ref.segment, "<")) {
            ApliNode node = vector_get(children, 1);
            return_value val1 = apli_evaluate_child(1);
            node = apli_get_child(2);
            return_value val2 = apli_evaluate_child(1);
            return_value ret;
            ret.type = NUMBER;
            ret.ref.num = NUMBER == val1.type && NUMBER == val2.type && val1.ref.num < val2.ref.num;
            return ret;
        } else if (seg_eq_str(id.ref.segment, ">")) {
            ApliNode node = vector_get(children, 1);
            return_value val1 = apli_evaluate_child(1);
            node = apli_get_child(2);
            return_value val2 = apli_evaluate_child(1);
            return_value ret;
            ret.type = NUMBER;
            ret.ref.num = NUMBER == val1.type && NUMBER == val2.type && val1.ref.num > val2.ref.num;
            return ret;
        } else if (seg_eq_str(id.ref.segment, "<=")) {
            ApliNode node = vector_get(children, 1);
            return_value val1 = apli_evaluate_child(1);
            node = apli_get_child(2);
            return_value val2 = apli_evaluate_child(1);
            return_value ret;
            ret.type = NUMBER;
            // printf("val1.ref.num = %d\n", val1.ref.num);
            // printf("val2.ref.num = %d\n", val2.ref.num);
            ret.ref.num = NUMBER == val1.type && NUMBER == val2.type && val1.ref.num <= val2.ref.num;
            return ret;
        } else if (seg_eq_str(id.ref.segment, ">=")) {
            ApliNode node = vector_get(children, 1);
            return_value val1 = apli_evaluate_child(1);
            node = apli_get_child(2);
            return_value val2 = apli_evaluate_child(1);
            return_value ret;
            ret.type = NUMBER;
            ret.ref.num = NUMBER == val1.type && NUMBER == val2.type && val1.ref.num >= val2.ref.num;
            return ret;
        } else if(seg_eq_str(id.ref.segment, "let")) {
            ApliNode bindings = vector_get(children, 1);
            // LOOP_OVER_REST_SEXPRS(bindings.children, )

        } else if(seg_eq_str(id.ref.segment, "if")) {
            ApliNode node = vector_get(children, 1);
            return_value comp = apli_evaluate_child(1);
            if(return_value_is_truthy(comp)) {
                node = apli_get_child(2);
                return apli_evaluate_child(1);
            } else {
                node = apli_get_child(2);
                node = apli_get_child(2);
                return apli_evaluate_child(1);
            }
        } else {
            printf("Invalid call! ");
            print_return_value(id);
            exit(1);
        }
    }
}

size_t return_value_is_truthy(return_value rv) {
    if(NUMBER == rv.type) {
        return rv.ref.num;
    } else {
        return 1;
    }
}

void* global_ptr = NULL;
const char *_segment_to_str(string_segment segment) {
    if(NULL != global_ptr)
        free(global_ptr);
    char *ptr = (char*) malloc(segment.length + 1);
    for(size_t i = 0; i < segment.length; ++i)
        ptr[i] = segment.str[i];
    ptr[segment.length] = '\0';
    global_ptr = ptr;
    return global_ptr;
}

string_segment _str_to_segment(const char* str, size_t len) {
    string_segment segment = {str, len};
    return segment;
}

size_t _segment_eq_str(string_segment segment, const char *str) {
    return seg_eq(segment, _str_to_segment(str, strlen(str)));
}

void print_return_value(return_value val) {
    if(NUMBER == val.type) {
        printf("NUMBER: %d\n", val.ref.num);
    }else if(IDENTIFIER == val.type) {
        printf("IDENTIFIER: `%s`\n", seg_to_str(val.ref.segment));
    }
}