# Abstract Programming Language Interpreter (APLI)
The Abstract Programming Language Interpreter (APLI) is a framework for generating and recursively walking Abstract Syntax Trees (ASTs). The user is responsible for working with the AST, walking through the nodes via `eval-hooks`, in order to interpret/compile/transpile the parsed output. 

## Why use APLI?
Although other lexing and parsing tools exist, many tools have users write code in a domain-specific language (ie. lex files or yacc files). DSLs can make the library: (1) harder to understand and (2) detract from the author's intent. \
\
APLI handles the lexing and parsing steps in a way that is simple and declarative. Everything is written natively in C. No confusing syntax -- all APLI api calls start with `apli`, and the user can choose to explicitly call the api with arguments or let APLI infer the names of the arguments.

A clear separation between the lexing & parsing steps (APLI) and the user's evaluator delineates responsibility. As the author of APLI I am responsible for making performance improvements and making sure that APLI is bug free, but, after the AST is constructed, any performance improvements are the user's responsibility.  

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

NOTE: Although these regexes correctly describe the tokens, APLI works with regexes that "contain the entire token" rather than performing a "largest match". The internal regex matching algorithm only returns the shortest matches. This means, a NUMBER would have to be written as `[^0-9][1-9][0-9]*[^0-9]` with a `+1` offset from the left and a `-1` offset from the right. 

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
        (NUMBER, "([^0-9]-[1-9][0-9]*[^0-9])", 1, 1),    // +1 left offset, -1 right offset
        (NUMBER, "([^0-9]\\+?[1-9][0-9]*[^0-9])", 1, 1), // ...
        (NUMBER, "[^0-9]0[^0-9]", 1, 1),                 // ...
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

Check out `calculator.c` in the `evaluators` folder to see a working implementation. 

If you wanted to write something more complex, the parser can parse left-to-right & right-to-left and works with a grammars with one look-ahead (multiple look-ahead is untested). Look at `lisp.c` for proof-of-concept. Is it fast? No. Does it work? Check out the integration tests (yes?).
