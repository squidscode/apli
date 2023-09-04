#! /bin/sh

LISP_FILE="./evaluators/lisp/lisp.c"
LISP_TEST_FILE="./resources/lisp/very_long_lisp_program.lisp"
INTEGRATION_PY="test/integration/test_lisp.py"
FLAGS="-DMULTITHREADED"

if [[ $1 == "parse-tree" ]]; then
    clang -O3 -DPRINT_PARSE_TREE $LISP_FILE;
elif [[ $1 == "parse-tree-steps" ]]; then
    clang -O3 -DPRINT_PARSE_TREE_STEPS -fsanitize=address -fno-omit-frame-pointer $LISP_FILE;
elif [[ $1 == "debug" ]]; then
    clang -g -DPRINT_STACK_FRAME -DPRINT_PARSE_TREE -DPRINT_LOOK_AHEAD_TREE -fsanitize=address -fno-omit-frame-pointer $LISP_FILE;
elif [[ $1 == "dry-run" ]]; then
    clang -Ofast $FLAGS -DDRY_RUN $LISP_FILE;
elif [[ $1 == "prof-test" ]]; then
    echo "Beginning to profile..."
    ./compile.sh profile
    echo "Running integration tests..."
    ./compile.sh integration-test
elif [[ $1 == "integration-test" ]]; then
    python3 $INTEGRATION_PY
elif [[ $1 == "profile" ]]; then
    clang -Ofast -DDRY_RUN -fprofile-instr-generate -fcoverage-mapping $LISP_FILE
    ./a.out $LISP_TEST_FILE
    llvm-profdata merge -output=merge.out -instr default.profraw
    llvm-cov show ./a.out  -instr-profile=merge.out > debug/coverage_info.txt
    rm ./a.out merge.out default.profraw
elif [[ $1 == "arena-allocated" ]]; then
    clang -Ofast -DARENA_ALLOCATOR $LISP_FILE;
elif [[ $1 == "" ]]; then
    clang -Ofast $FLAGS $LISP_FILE;
else
    printf "Invalid arguments provided.\n"
fi
