# Abstract Programming Language Interpreter (APLI)
The Abstract Programming Language Interpreter (APLI) is a customizable interpreter that reads a `language config file` and and an `instruction file` and executes the instructions in the described language.

Execute the following command to see the lexer in action:
```bash
$ clang -D COLOR -Ofast test/lexer/lex_visual_test.c; \
    ./a.out; \
    rm ./a.out;
```