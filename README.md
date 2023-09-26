# Abstract Programming Language Interface (APLI)
The Abstract Programming Language Interface (APLI) is a framework for generating and recursively walking Abstract Syntax Trees (ASTs). The user is responsible for working with the AST by walking through the nodes via node-visitors (called `apli-functions`) in order to interpret/compile/transpile the parsed output. 

## Why use APLI?
APLI handles the lexing and parsing steps in a way that is simple and declarative. Everything is written natively in C. No confusing syntax -- all APLI api calls start with `apli`, and the user can choose to explicitly call the api with arguments or let APLI infer the names of the arguments.

A clear separation between the lexing & parsing steps (APLI) and the user's "evaluation code" delineates responsibility (this happens to be the separation between the **syntax** and the **semantics** of a programming language). You (the user) can worry about the semantics of your programming language and delegate the syntax parsing to APLI.

APLI is also very fast. Currently, it can lex and parse at a speed of about 4 Mb/s. If you decide to evaluate a small file, you can get AST parsing done in about 15 milliseconds. If you embed your regular expression DFA into your source file (see lisp example) you can get startup times of 5 milliseconds on small files. 

## How do I write an evaluator?
The steps to construct an evaluator are:

1. Create or find the languages BNF rules. Here's an example of a BNF for simple arithmetic expressions:
```
<expr>   := <term>
<expr>   := <expr> ('+' | '-') <term>
<term>   := <factor>
<term>   := <term> ('*' | '/') <factor>
<factor> := NUMBER | '(' <expression> ')' | <factor> '^' NUMBER
```

2. Construct a list of regexes you need to tokenize the input.
```
NUMBER   := "[1-9][0-9]*"
PLUS     := "\\+"
MINUS    := "-"
STAR     := "\\*"
F_SLASH  := "/"
OP_PAREN := "\\("
CL_PAREN := "\\)"
CARET    := "\\^"
```

3. Construct your evaluator! \
  (a) Include "<apli.h>". \
  (b) Define the required eval-hook macros and call `apli_init`. \
  (c) Forward declare *apli_functions* corresponding to the non-terminals in your BNF. \
  (d) Define your non-terminals and terminals. \
  (e) Map each terminal to a regex using `apli_regex` and compile your regexes. \
  (f) Write out your BNF rules using `apli_bnf`.
  (g) Call `apli_evaluate` on your input (of type `char *`).

```c
#include <apli.h>

#define APLI_EVAL_ARGUMENTS
#define APLI_EVAL_NAMES
#define APLI_EVAL_RETURN_TYPE int

apli_init();
apli_define_functions(expr, term, factor);

__APLI_START__
    if(2 != argc)
        assert(0 == "The second argument must be the arithmetic expression");

    apli_non_terminals(expr, term, factor);
    apli_terminals(NUMBER, PLUS, MINUS, STAR, FORWARD_SLASH, OPEN_PAREN, CLOSE_PAREN, CARET);

    // Regexes need to be written in order of precedence!
    apli_regex(
    //  (terminal_name, regex, pre-offset=0, post-offset=0)
        (NUMBER, "[1-9][0-9]*"),
        (PLUS, "\\+"),
        (MINUS, "-"),
        (STAR, "\\*"),
        (CARET, "\\^"),
        (FORWARD_SLASH, "/"),
        (OPEN_PAREN, "\\("),
        (CLOSE_PAREN, "\\)")
    );
    apli_regex_compile();

    // BNF rules.
    apli_bnf(
    //  (left-hand side rule, [=] rule #1, rule #2 ...)
        (expr, term),
        (expr, expr, PLUS, term),
        (expr, expr, MINUS, term),
        (term, factor),
        (term, term, STAR, factor),
        (term, term, FORWARD_SLASH, factor),
        (factor, NUMBER),
        (factor, factor, CARET, NUMBER),
        (factor, OPEN_PAREN, expr, CLOSE_PAREN)
    );

    // Add a buffer before and after to accomodate for regex offsets.
    char *input = add_pre_post_buffer(argv[1], 1); 

    // Print the evaluation result.
    printf("%i\n", apli_evaluate(argv[1]));

    free(input);

__APLI_END__

apli_function(expr) {
    // Your code here!
}

apli_function(term) {
    // Your code here!
}

apli_function(factor) {
    // Your code here!
}
```

Check out [`calculator.c`](evaluators/arithmetic/calculator.c) to see a working implementation. Also, check out [`lisp.c`](evaluators/lisp/lisp.c) for a tree-walking lisp interpreter. It can currently interpret [the following files](test/integration/resources/simple), and is approximately 10 times slower than `clisp`, partly due to inefficiencies in `apli` and, probably, the fact that it's not complied to bytecode first. 

If you wanted to write something more complex, the parser can parse left-to-right & right-to-left and works with a grammars with one look-ahead (multiple look-ahead is untested). Look at `lisp.c` for a simple tree-walking interpreter for inspiration. 

# Lisp Evaluator Boilerplate Code
For more complicated evaluators, you might want to re-parse a previous node. For that, you'll have to use the `ApliNode` structure. `ApliNode`s are references to nodes on the AST. To evaluate the node, you can call `apli_evaluate_args` with the apli node you want to evaluate to dynamically dispatch to the correct `apli_function`. 

I'd suggest looking at the `apli.h` file for macros implementation. Documentation has not been written yet.

```c
#include <apli.h>

#define APLI_EVAL_ARGUMENTS   environment *env
#define APLI_EVAL_NAMES       env
#define APLI_EVAL_RETURN_TYPE return_value


#define resolve_id(env, id)         _resolve_identifier(env, id)
#define env_new()                   _env_new()
#define env_free(env)               _env_free(env)
#define push_frame(env)             _push_frame(env)
#define pop_frame(env)              _pop_frame(env)
#define extend_env(env, id, val)    _extend_env(env, id, val)


// --------------------------------------
// ---------- Data definitions ----------
// --------------------------------------

typedef struct _identifier {
    const char *str;
    size_t length;
} identifier;

typedef Map(identifier, return_value)* frame;

typedef struct _environment {
    frame *stack_frame;
} environment;

typedef struct _function_value {
    environment *closure;
    ApliNode function_pointer;
    identifier *arguments;
} function_value;

typedef union _rv_data {
    int num;
    const char *segment;
    function_value fun_v;
} rv_data;

typedef enum _rv_type {NUMBER, IDENTIFIER, FUNCTION, STRING} rv_type;

typedef struct _return_value_type {
    rv_type type;
    rv_data ref;
} return_value;

// --------------------------------------


apli_init();
apli_define_functions(s_expression, list, s_expressions, atomic_symbol);

__APLI_START__
    // Since we rely on right-recursion in our bnf rules, we need to tell the
    // parser to parse right-to-left.
    apli_set_parser_type(RIGHT_TO_LEFT);

    apli_non_terminals(s_expression, list, s_expressions, atomic_symbol);
    apli_terminals(ATOMIC_SYMBOL, OPEN_PAREN, CLOSE_PAREN, PERIOD, COMMENT);

    apli_regex(
        (COMMENT,       ";[^\n]*"),
        (ATOMIC_SYMBOL, "(\"([^\n\"]|\\\")*\"|[a-z0-9\\-]+|(<=|>=|[+-\\*/<>=]))"),
        (OPEN_PAREN,    "\\("),
        (CLOSE_PAREN,   "\\)"),
        (PERIOD,        "\\.")
    );
    apli_regex_compile();

    apli_bnf(
        (s_expression,  atomic_symbol),
        (s_expression,  OPEN_PAREN, s_expression, PERIOD, s_expression, CLOSE_PAREN),
        (s_expression,  list),
        (list,          OPEN_PAREN, s_expressions, CLOSE_PAREN),
        (list,          OPEN_PAREN, CLOSE_PAREN),
        (s_expressions, s_expression),
        (s_expressions, s_expression, s_expressions),
        (atomic_symbol, ATOMIC_SYMBOL)
    );

    // A better abstraction for these steps will be coming soon.
    List(_token_t) *tokens = token_rules_tokenize(token_rules, input);
    _token_rules_ignore_token(tokens, "COMMENT");        
    parse_tree_result = bnf_rules_construct_parse_tree(bnf_rules, tokens, parser_type_inst);

    environment *env = env_new();
    push_frame(env);                              // the global stack frame
    apli_evaluate_node(parse_tree_result.root);
    env_free(env);

    free(input);

__APLI_END__

apli_define_functions(s_expression_case_1, s_expression_case_2);
apli_function(s_expression) {
    // ...
}
apli_function(s_expression_case_1) {
    // ...
}
apli_function(s_expression_case_2) {
    // ...
}

apli_function(list) {
    // ...
}

apli_function(s_expressions) {
    // ...
}

apli_function(atomic_symbol) {
    // ...
}

// Imlementation details...
```

# Caveats
APLI doesn't do well with ambiguous grammars because it can only construct a single-pass parser. It relies on the user to write syntax rules that aren't ambiguous. This is an issue I will be fixing in the future by creating a recursive parser, but it hasn't been written yet. Write your BNF rules with caution!
