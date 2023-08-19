#! /bin/sh

LISP_FILE="./evaluators/lisp/lisp.c"

if [[ $1 == "parse-tree" ]]; then
    clang -O3 -DPRINT_PARSE_TREE $LISP_FILE;
elif [[ $1 == "parse-tree-steps" ]]; then
    clang -O3 -DPRINT_PARSE_TREE_STEPS -fsanitize=address -fno-omit-frame-pointer $LISP_FILE;
elif [[ $1 == "debug" ]]; then
    clang -g -DPRINT_STACK_FRAME -DPRINT_PARSE_TREE -DPRINT_LOOK_AHEAD_TREE -fsanitize=address -fno-omit-frame-pointer $LISP_FILE;
else
    clang -Ofast $LISP_FILE;
fi
