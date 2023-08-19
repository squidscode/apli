#! /bin/sh

if [[ $1 == "parse-tree" ]]; then
    clang -O3 -DPRINT_PARSE_TREE ./test/lisp.c;
elif [[ $1 == "parse-tree-steps" ]]; then
    clang -O3 -DPRINT_PARSE_TREE_STEPS -fsanitize=address -fno-omit-frame-pointer ./test/lisp.c;
elif [[ $1 == "debug" ]]; then
    clang -O3 -DPRINT_PARSE_TREE -DPRINT_LOOK_AHEAD_TREE -fsanitize=address -fno-omit-frame-pointer ./test/lisp.c;
else
    clang -Ofast ./test/lisp.c;
fi
