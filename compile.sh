#! /bin/sh

if [[ $1 == "parse-tree" ]]; then
    clang -Ofast -DPRINT_PARSE_TREE ./test/lisp.c;
else
    clang -Ofast ./test/lisp.c;
fi