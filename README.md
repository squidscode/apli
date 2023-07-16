# Abstract Programming Language Interpreter (APLI)
The Abstract Programming Language Interpreter (APLI) is a customizable interpreter that reads a `language config file` and and an `instruction file` and executes the instructions in the described language.

Execute the following command to see the lexer in action:
```bash
$ clang -D DEBUG -D COLOR -D MEMORY_CHECK -g -fsanitize=address -fno-omit-frame-pointer test/lexer/lex_test.c ; ./a.out; rm a.out;
```