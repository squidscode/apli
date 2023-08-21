#include "../../src/apli.h"

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
 * list          := "(" ")"
 * s_expressions := s_expression s_expressions       ** NOTICE: s_expressions == s_expression+
 * s_expressions := s_expression
 * atomic_symbol := r"[^a-z1-9][a-z1-9]+[^a-z1-9]" [OFFSET: +1 ; -1]
 */

typedef struct _string_segment {
    const char *str;
    size_t length;
} string_segment;

typedef string_segment identifier;
typedef struct _return_value_type return_value;

typedef struct _return_value_type return_value;
typedef struct _identifier_return_value_map_ _identifier_return_value_map_t;
typedef Map(identifier, return_value)* frame;
typedef struct _frame_vector_ Vector(frame);
typedef struct _identifier_vector_ Vector(identifier);

typedef struct _environment {
    Vector(frame) *stack_frame;
} environment;

typedef struct _function_value {
    environment *closure;
    ApliNode function_pointer;
    Vector(identifier) *arguments;
} function_value;

// NONE is used as a placeholder for implementing recursively defined functions.
typedef enum _rv_type {NUMBER, IDENTIFIER, FUNCTION, STRING} rv_type;
/**
 * NUMBER -> ref is size_t
 * IDENTIFIER -> ref is string_segment
 */
typedef union _rv_data {
    int num;
    string_segment segment;
    function_value fun_v;
} rv_data;

typedef struct _return_value_type {
    rv_type type;
    rv_data ref;
} return_value;

define_map(identifier, return_value);
define_vector(frame);
define_vector(identifier);

#define resolve_id(env, id)         _resolve_identifier(env, id)
#define env_new()                   _env_new()
#define env_free(env)               _env_free(env)
#define push_frame(env)             _push_frame(env)
#define pop_frame(env)              _pop_frame(env)
#define extend_env(env, id, val)    _extend_env(env, id, val)
#define clone_env(env)              _clone_env(env)
#define seg_to_str(seg)             _segment_to_str(seg)
#define seg_eq_str(seg, str)        _segment_eq_str(seg, str)
#define str_to_seg(str, len)        _str_to_segment(str, len)

const char* ftoca(const char* file_path);
return_value _resolve_identifier(environment *env, string_segment id);
environment *_env_new();
void _env_free(environment *env);
void _push_frame(environment *env);
void _pop_frame(environment *env);
void _extend_env(environment *env, string_segment id, return_value rv);
environment *_clone_env(environment *env);

size_t seg_hash(string_segment seg);
size_t seg_eq(string_segment seg1, string_segment seg2);
const char *_segment_to_str(string_segment segment);
string_segment _str_to_segment(const char* str, size_t len);
size_t _segment_eq_str(string_segment segment, const char *str);

apli_init();
apli_define_functions(s_expression, list, s_expressions, atomic_symbol);

char *add_pre_post_buffer(const char *str, size_t buffer_size);
void print_return_value(return_value val);

#define print_env(env) \
    printf("@<%p> (", env); \
    print_frame(vector_get(env->stack_frame, 0)); \
    for(size_t i = 1; i < vector_size(env->stack_frame); ++i) { \
        printf(", "); print_frame(vector_get(env->stack_frame, i)); \
    } \
    printf(") ")
#define print_string_segment(seg) \
    printf("`"); \
    for(size_t IND = 0; IND < seg.length; ++IND) { \
        printf("%c", seg.str[IND]); \
    } \
    printf("`")

void print_return_type(return_value val);
void print_frame(frame f);


__APLI_START__
    // If we didn't parse right to left, then the parser errors on "(A B C)". This is due to the
    // lack of a `s_expressions := s_expressions s_expressions` rule. See the README for more information.
    // I'd suggest you try to reason about it yourself! Use the `-DPRINT_PARSE_TREE` or `-DPRINT_PARSE_TREE_STEPS`
    // compiler flags and comment out the line below to use the default `LEFT_TO_RIGHT` parser.
    apli_set_parser_type(RIGHT_TO_LEFT);

    if(argc < 2 || 3 < argc)
        assert(0 == "Invalid # of arguments to executable.");

    apli_non_terminals(s_expression, list, s_expressions, atomic_symbol);
    apli_terminals(ATOMIC_SYMBOL, OPEN_PAREN, CLOSE_PAREN, PERIOD);

    apli_regex(
        (ATOMIC_SYMBOL, "[^\\]\"[^\n]*[^\\]\"", 1, 0),
        (ATOMIC_SYMBOL, "[^a-z0-9\\-][a-z0-9\\-]+[^a-z0-9\\-]", 1, 1),
        (ATOMIC_SYMBOL, "(<=|>=)", 0, 0),
        (ATOMIC_SYMBOL, "([+-\\*/<>=])", 0, 0),
        (OPEN_PAREN, "\\("),
        (CLOSE_PAREN, "\\)"),
        (PERIOD, ".")
    );
    apli_regex_compile();


    apli_bnf(
        (s_expression, atomic_symbol),
        (s_expression, OPEN_PAREN, s_expression, PERIOD, s_expression, CLOSE_PAREN),
        (s_expression, list),
        (list, OPEN_PAREN, s_expressions, CLOSE_PAREN),
        (list, OPEN_PAREN, CLOSE_PAREN),
        (s_expressions, s_expression),
        (s_expressions, s_expression, s_expressions),
        (atomic_symbol, ATOMIC_SYMBOL)
    );

    char *input;
    if(0 == strcmp("-e", argv[1]) || 0 == strcmp("--execute", argv[1])) {
        if(3 != argc) {
            printf("A second argument was not provided.\n");
            exit(1);
        }
        input = add_pre_post_buffer(argv[2], 2);
    } else {
        if(2 != argc) {
            printf("Invalid arguments provided to executable.\n");
            exit(1);
        }
        input = add_pre_post_buffer(ftoca(argv[1]), 2);
    }

    parse_tree_result = apli_get_parse_tree((input), parser_type_inst);

    // DRY_RUN wil only run the lexing and parsing steps. Since the evaluation is the
    // user's reponsibility, I will be focusing on optimizing the dry run.
#ifdef DRY_RUN
    exit(0);
#endif

    environment *env = env_new();
    push_frame(env);
    apli_evaluate_node(parse_tree_result.root);
    env_free(env);

    free(input);

__APLI_END__

environment *_env_new() {
    environment *env = (environment*) malloc(sizeof(environment));
    env->stack_frame = vector_new(frame);
    return env;
}

void _env_free(environment *env) {
    while(vector_size(env->stack_frame))
        pop_frame(env);
    vector_free(env->stack_frame);
    free(env);
}

apli_function(s_expressions) {
    // printf("s_expressions\n");
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
    // printf("s_expression\n");
    if(apli_child_terminal_name_equals(atomic_symbol, 1)) {
        // s_expression = atomic_symbol
        return apli_eval_child(1);
    } else if(apli_child_token_name_equals(OPEN_PAREN, 1)) {
        // s_expression = "(" s_expression "." s_expressison ")"
        assert(0 == "Not implemented!");
    } else {
        // s_expression = list
        return apli_eval_child(1);
    }
    assert(0 == "Not reachable");
}

apli_function(atomic_symbol) {
    // printf("atomic_symbol\n");
    // atomic_symbol := r"[^a-z1-9][a-z1-9]+[^a-z1-9]" [OFFSET: +1 ; -1]
    ApliToken tok = apli_get_child_token(1);
    string_segment segment = {apli_token_ref(tok), apli_token_reflen(tok)};
    return_value rv;
    if('0' <= apli_token_ref(tok)[0] && apli_token_ref(tok)[0] <= '9') {
        rv.type = NUMBER;
        rv.ref.num = atoi(seg_to_str(segment));
        return rv;
    } else if(('"' == apli_token_ref(tok)[0]) && ('"' == apli_token_ref(tok)[segment.length - 1])) {
        rv.type = STRING;
        rv.ref.segment = segment;

        // offset the string:
        rv.ref.segment.str += 1;
        rv.ref.segment.length -= 2;

        return rv;
    } else {
        return resolve_id(env, segment);
    }
    
    assert(0 == "Not reachable");
}

return_value lisp_call(return_value id, Vector(_parse_tree_node_t) *children, environment*);

apli_function(list) {
    // printf("list\n");
    // list          := "(" s_expressions ")"
    if(3 == apli_num_children()) {
        ApliNode sexprs = apli_get_child(2);
        return lisp_call(apli_evaluate_node(vector_get(sexprs.children, 0)), sexprs.children, env);
    }
    printf("Evaluating '()' is not possible!\n");
    assert(0 == "Invalid evaluation state!");
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

    // printf("Stack size: %zu\n", sz);
    for(size_t i = sz - 1; i < sz; --i) {
        // Todo the following if statements are for debugging!
        // if(vector_get(sf, i)->hash != &seg_hash)
        //     assert(0 == "Wrong hash function encoded!");
        // if(vector_get(sf, i)->key_eq != &seg_eq)
        //     assert(0 == "Wrong key function encoded!");
        // printf("Map size: %zu\n", map_size(vector_get(sf, i)));
        if(map_count(vector_get(sf, i), id)) {
            // printf("HIT!!");
            return map_at(vector_get(sf, i), id);
        }
    }
    return_value rv;
    rv.type = IDENTIFIER;
    rv.ref.segment = id;
    return rv;
    // printf("Invalid Identifier Error! Identifier `%s` is not bound.\n", seg_to_str(id));
    // assert(0 == "Invalid identifier");
}

size_t seg_hash(string_segment seg) {
    size_t hash = 0UL;
    size_t mod = sizeof(size_t) / sizeof(char);
    size_t offset = 0;
    // printf("hashing ... "); print_string_segment(seg); 
    for(size_t i = 0; i < seg.length; ++i) {
        // printf(".. `%c` .", ptr[i]);
        hash ^= ((255UL & seg.str[i]) << (8 * offset++));
        offset %= mod;
    }
    // printf(" has hash: %zu\n", hash);
    return hash; 
}

size_t seg_eq(string_segment seg1, string_segment seg2) {
    // printf("Comparing: "); print_string_segment(seg1); printf(" "); print_string_segment(seg2);
    // printf("\n");
    if(seg1.length != seg2.length)
        return 0;
    for(size_t i = 0; i < seg1.length; ++i)
        if(seg1.str[i] != seg2.str[i])
            return 0;
    // printf("TRUE!\n");
    return 1;
}

void _push_frame(environment *env) {
    frame f = map_new(identifier, return_value);
    map_set_hash(f, &seg_hash);
    map_set_key_eq(f, &seg_eq);
    vector_push_back(env->stack_frame, f);
}

void _pop_frame(environment *env) {
    assert(0 < vector_size(env->stack_frame));
    map_free(vector_get_back(env->stack_frame));
    vector_pop_back(env->stack_frame);
}

void _extend_env(environment *env, string_segment id, return_value rv) {
#ifndef NO_ID_BINDING_WARN
    if(IDENTIFIER == rv.type)
        (assert(0 == "Warning! Binding value is an identifier!"));
#endif
    frame f = vector_get_back(env->stack_frame);
    map_insert(f, id, rv);
}

environment *_clone_env(environment *env) {
    size_t sz = vector_size(env->stack_frame);
    environment *new_env = env_new();
    for(size_t i = 0; i < sz; ++i) {
        frame nxt_frame = vector_get(env->stack_frame, i);
        frame frame_clone = map_clone(nxt_frame);
        vector_push_back(new_env->stack_frame, frame_clone);
    }
    return new_env;
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
void map_bindings(ApliNode node, environment *env);
Vector(identifier) *construct_list_of_args(ApliNode args_node, environment *env);


return_value lisp_call(return_value id, Vector(_parse_tree_node_t) *children, environment *env) {
    if(IDENTIFIER == id.type) // used to resolve recursive identifiers.
        id = resolve_id(env, id.ref.segment);
#ifdef PRINT_STACK_FRAME
    printf("\x1b[31;1m");
    print_return_value(id); printf(" "); print_env(env); 
    printf("\x1b[0m\n");
#endif

    if(NUMBER == id.type) {
        printf("Number `%d` is not callable.\n", id.ref.num);
        exit(1);
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
            ApliNode node = vector_get(children, 1);
            ApliNode bindings = apli_get_child(1);
            node = apli_get_child(2);

            push_frame(env);
            map_bindings(bindings, env);
            return_value body_evaluation = apli_evaluate_node(node);
            // printf("BODY: "); print_return_value(resolve_id(env, body_evaluation.ref.segment));
            pop_frame(env);
            return body_evaluation;
        } else if(seg_eq_str(id.ref.segment, "defun")) {
            ApliNode node = vector_get(children, 1);
            ApliNode function_name = apli_node_get_child(apli_get_child(1), 1);
            if(!apli_node_terminal_name_equals(function_name, atomic_symbol))
                assert(0 == "Function name must be an atomic_symbol");
            return_value function_name_rv = apli_evaluate_node(function_name);
            assert(IDENTIFIER == function_name_rv.type);
            identifier function_name_id = function_name_rv.ref.segment;

            node = apli_get_child(2);
            ApliNode args_node = apli_get_child(1);
            ApliNode function_body = apli_get_child(2);

            environment *nenv = env_new();
            Vector(identifier) *identifier_vec = construct_list_of_args(args_node, nenv);
            env_free(nenv);

            return_value rv;
            rv.type = FUNCTION;

            rv.ref.fun_v.closure = clone_env(env);
            rv.ref.fun_v.function_pointer = function_body;
            rv.ref.fun_v.arguments = identifier_vec;

            // print_env(env);
            // print_env(rv.ref.fun_v.closure);
            extend_env(rv.ref.fun_v.closure, function_name_id, rv);
            extend_env(env, function_name_id, rv);

            // printf("Function made "); print_return_value(rv);
            // print_env(env);
            // print_env(rv.ref.fun_v.closure);

            return rv;
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
        } else if(seg_eq_str(id.ref.segment, "lambda")) {
            ApliNode node = vector_get(children, 1);
            ApliNode args_node = apli_get_child(1);
            ApliNode function_body = apli_get_child(2);

            environment *nenv = env_new();
            Vector(identifier) *identifier_vec = construct_list_of_args(args_node, nenv);
            env_free(nenv);

            return_value rv;
            rv.type = FUNCTION;

            rv.ref.fun_v.closure = clone_env(env);
            rv.ref.fun_v.function_pointer = function_body;
            rv.ref.fun_v.arguments = identifier_vec;

            return rv;
        } else if(seg_eq_str(id.ref.segment, "funcall")) {
            ApliNode node = vector_get(children, 1);
            ApliNode function_name = apli_node_get_child(apli_get_child(1), 1);
            if(!apli_node_terminal_name_equals(function_name, atomic_symbol))
                assert(0 == "Function name must be an atomic_symbol");
            return_value function_name_rv = apli_evaluate_node(function_name);
            return lisp_call(function_name_rv, node.children, env);
        } else if(seg_eq_str(id.ref.segment, "terpri")) {
            printf("\n"); // NOTE: Assume linux.
            return_value one;
            one.type = NUMBER;
            one.ref.num = 1;
            return one; 
        } else if(seg_eq_str(id.ref.segment, "write")
            || seg_eq_str(id.ref.segment, "write-string")
            || seg_eq_str(id.ref.segment, "write-line")) {
            ApliNode node = vector_get(children, 1);
            return_value rv = apli_evaluate_node(node);
            if(NUMBER == rv.type) {
                printf("%d", rv.ref.num);
                return rv;
            } else if(STRING == rv.type) {
                string_segment seg = rv.ref.segment;
                for(size_t i = 0; i < seg.length; ++i) {
                    if('\\' == seg.str[i] && i + 1 < seg.length) {
                        switch(seg.str[i + 1]) {
                        case 'n':
                            printf("\n");
                            i += 1;
                            break;
                        case 'r':
                            printf("\r");
                            i += 1;
                            break;
                        case 't':
                            printf("\t");
                            i += 1;
                            break;
                        default:
                            printf("\\");
                            break;
                        }
                        continue;
                    }
                    printf("%c", seg.str[i]);
                }
                if(seg_eq_str(id.ref.segment, "write-line"))
                    printf("\n");
                return rv;
            } else {
                printf("Cannot print invalid type "); print_return_type(rv);
                printf("\n");
                exit(1);
            }
        } else if(seg_eq_str(id.ref.segment, "progn")) {
            return_value one;
            one.type = NUMBER;
            one.ref.num = 1;
            if(1 == vector_size(children))
                return one;
            return apli_evaluate_node(vector_get(children, 1));
        } else if(seg_eq_str(id.ref.segment, "and")) {
            return_value rv;
            if(1 < vector_size(children)) {
                rv.type = NUMBER;
                ApliNode node = vector_get(children, 1);
                while(apli_node_terminal_name_equals(node, s_expressions)) {
                    return_value result = apli_evaluate_child(1);
                    if(NUMBER == result.type && 0 == result.ref.num) {
                        rv.ref.num = 0;
                        return rv;
                    }
                    if(vector_size(node.children) < 2)
                        break;
                    node = apli_get_child(2);
                }
            }
            rv.ref.num = 1;
            return rv;
        } else if(seg_eq_str(id.ref.segment, "or")) {
            return_value rv;
            if(1 < vector_size(children)) {
                rv.type = NUMBER;
                ApliNode node = vector_get(children, 1);
                while(apli_node_terminal_name_equals(node, s_expressions)) {
                    return_value result = apli_evaluate_child(1);
                    if(NUMBER != result.type || 0 != result.ref.num) {
                        rv.ref.num = 1;
                        return rv;
                    }
                    if(vector_size(node.children) < 2)
                        break;
                    node = apli_get_child(2);
                }
            }
            rv.ref.num = 0;
            return rv;
        } else {
            printf("Invalid call! ");
            print_env(env);
            print_return_value(id);
            exit(1);
        }
    } else if(FUNCTION == id.type) {
        // printf("Function call "); print_env(env); printf("\n");
        environment *tmp_closure = id.ref.fun_v.closure;
        push_frame(tmp_closure);
        // print_env(tmp_closure);
        size_t arg_size = vector_size(id.ref.fun_v.arguments);
        ApliNode node;
        if(1 < vector_size(children))
            node = vector_get(children, 1); // sexprs
        for(size_t i = 0; i < arg_size; ++i) {
            extend_env(tmp_closure, vector_get(id.ref.fun_v.arguments, i), apli_evaluate_child(1));
            if(i == arg_size - 1)
                break;
            if(1 == apli_num_children())
                assert(0 == "Invalid # of arguments given to function call.");
            node = apli_get_child(2);
        }
        if(1 <= arg_size && 1 != apli_num_children())
            assert(0 == "Invalid # of arguments given to function call.");
        return_value rv = apli_evaluate_node_args(id.ref.fun_v.function_pointer, tmp_closure);
        pop_frame(tmp_closure);
        return rv;
    }

    printf("Return value is not callable! ");
    print_return_value(id);
    exit(1);
}

size_t return_value_is_truthy(return_value rv) {
    if(NUMBER == rv.type) {
        return rv.ref.num;
    } else {
        return 1;
    }
}

void map_bindings(ApliNode node, environment *env) {
    // node is an s_expression
    node = apli_get_child(1); // node : list
    // _parser_print_parse_tree_value(node.root);
    if(!apli_node_terminal_name_equals(node, list))
        (assert(0 == "Bindings must be a list."));

    if(3 != apli_num_children()) 
        return;
    node = apli_get_child(2); // s_expressions
    while(1) {
        ApliNode binding = apli_get_child(1);
        binding = apli_node_get_child(binding, 1);
        if(!apli_node_terminal_name_equals(binding, list))
            (assert(0 == "Bindings must be a list."));
        if(3 != vector_size(binding.children))
            (assert(0 == "Bindings cannot be '()'"));
        binding = apli_node_get_child(binding, 2);

        ApliNode atomic_symbol_id = apli_node_get_child(apli_node_get_child(binding, 1), 1);
        if(!apli_node_terminal_name_equals(atomic_symbol_id, atomic_symbol))
            (assert(0 == "Binding name must be an atomic_symbol!"));

        ApliToken tok = apli_node_get_child(atomic_symbol_id, 1).root.ptr.token;
        string_segment segment = {apli_token_ref(tok), apli_token_reflen(tok)};

        return_value var_name;
        var_name.type = IDENTIFIER;
        var_name.ref.segment = segment;
        return_value value = apli_evaluate_node_args(apli_node_get_child(binding, 2), env);


        if(IDENTIFIER != var_name.type)
            (assert(0 == "Binding name is not an identifier!"));
        string_segment var_segment = var_name.ref.segment;

        extend_env(env, var_segment, value);
        if(vector_size(node.children) < 2)
            break;
        node = apli_get_child(2);
    }
}

Vector(identifier) *construct_list_of_args(ApliNode node, environment *env) {
    // node is an s_expression
    node = apli_get_child(1); // node : list
    Vector(identifier) *ids = vector_new(identifier);

    // _parser_print_parse_tree_value(node.root);
    if(!apli_node_terminal_name_equals(node, list))
        (assert(0 == "Bindings must be a list."));

    if(3 != apli_num_children()) 
        return ids;
    node = apli_get_child(2); // s_expressions

    while(1 <= apli_num_children()) {
        ApliNode next_id = apli_node_get_child(apli_get_child(1), 1);
        if(!apli_node_terminal_name_equals(next_id, atomic_symbol))
            (assert(0 == "Bindings must be a list."));

        ApliToken tok = apli_node_get_child(next_id, 1).root.ptr.token;
        string_segment segment = {apli_token_ref(tok), apli_token_reflen(tok)};

        vector_push_back(ids, segment);
        if(1 == apli_num_children())
            break;
        node = apli_get_child(2);
    }
    return ids;
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
        printf("NUMBER: %d", val.ref.num);
    }else if(IDENTIFIER == val.type) {
        printf("IDENTIFIER: `%s`", seg_to_str(val.ref.segment));
    } else if(FUNCTION == val.type) {
        printf("FUNCTION <%p>", val.ref.fun_v.closure);
    }
}

void print_return_type(return_value val) {
    if(NUMBER == val.type) {
        printf("NUMBER");
    }else if(IDENTIFIER == val.type) {
        printf("IDENTIFIER");
    } else if(FUNCTION == val.type) {
        printf("FUNCTION");
    }
}

void print_frame(frame f) {
    _identifier_return_value_map_match_t_list_t *lst = map_get_list(f);
    printf("{");
    while(list_size(lst)) {
        MapMatch(identifier, return_value) nxt = list_get_front(lst);
        print_string_segment(nxt.key);
        printf(" ");
        print_return_value(nxt.value);
        if(1 != list_size(lst))
            printf(", ");
        list_pop_front(lst);
    }
    printf("}");
    
}

const char* ftoca(const char* file_path) {
    FILE *fp;
    fp = fopen(file_path, "r");
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    char *buff = (char*) malloc(sz+1);
    fseek(fp, 0L, SEEK_SET);
    for(size_t i = 0; i < sz; ++i) {
        buff[i] = fgetc(fp);
    }
    buff[sz] = '\0';
    return buff;
}