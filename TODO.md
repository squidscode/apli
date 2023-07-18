## Task List

**Define the format for a `.config` file (file suffix name subject to change)**
1. Define the lexing rules format
    - How will the user specify how to tokenize the instruction file?
    - The user should be allowed to *provide a regular expression to match* or *manually encode the Finite State Machine that matches tokens*. 
2. Define the parsing rules format

Useful links:
- [List of keywords in python](https://www.programiz.com/python-programming/keyword-list)
- [Guide to lexing and parsing](https://tomassetti.me/guide-parsing-algorithms-terminology/#:~:text=The%20definitions%20used%20by%20lexers,corresponds%20to%20a%20sum%20expression.)

Extensions:
- Is it possible to make a "un-parser" that converts an AST into another language? Perhaps, if this is possible, it might be possible to transpile any language supported by the APLI to another language that is supported by the APLI.

# For debugging purposes only:
```bash
clang -D DEBUG -D COLOR -D MEMORY_CHECK -fsanitize=address -fno-omit-frame-pointer -O2 test/lexer/lex_test.c ; ./a.out; rm a.out;
```