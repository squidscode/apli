from os import walk
import scipy.stats as scistats
import numpy as np
import subprocess as sb
import time
import shlex
from colors import prRed, prCyan

LEAF_NODE_NAME = "__LEAF__"
INDENT_STR = "  "
TOP_BOX_CHARACTER = "\u2514"
TEST_FILE_TIME_LIMIT = 5       # seconds
ALPHA = 0.05
ROUND_DIGITS = 3
MIN_TESTS = 5

# Use DRY_RUN option
sb.run(shlex.split("./compile.sh dry-run"))

# Construct the test tree
tests = {} # a tree.
w = walk('./test/integration/resources')
for (dirpath, dirnames, filenames) in w:
    layer = dirpath.count("/") - 3
    dir_array = dirpath.split("/")[3:]
    tree_ptr = tests
    for dir_name in dir_array:
        assert dir_name is not LEAF_NODE_NAME
        if dir_name in tree_ptr:
            tree_ptr = tree_ptr[dir_name]
        else:
            tree_ptr[dir_name] = {}
            tree_ptr = tree_ptr[dir_name]
    tree_ptr[LEAF_NODE_NAME] = list(map(lambda filename : dirpath + "/" + filename, filenames))

# Run each of the tests.
def start_test_runner() -> None:
    prCyan("resources")
    recursive_test_runner(tests['resources'])

def recursive_test_runner(test_tree, indent: int = 0) -> None:
    for test_path in test_tree[LEAF_NODE_NAME]:
        print_indent(indent + 1)
        print(TOP_BOX_CHARACTER, end="")
        print(" ", end="")
        prRed(test_path.split("/")[-1], end=" ")
        print_test(test_path)    
    for next_dirname in test_tree.keys():
        if next_dirname == LEAF_NODE_NAME:
            continue
        print_indent(indent + 1)
        print(TOP_BOX_CHARACTER, end="")
        print(" ", end="")
        prCyan(next_dirname)
        recursive_test_runner(test_tree[next_dirname], indent+1)

def print_indent(indent: int) -> None:
    for i in range(indent): 
        print(INDENT_STR, end="")

def print_test(test_path: str) -> None:
    # discard the first test for caching purposes
    COMMAND = "./a.out " + test_path
    first_test_time = run_shell_timed(COMMAND)
    num_tests = max(int(TEST_FILE_TIME_LIMIT / first_test_time - 1), MIN_TESTS)
    test_results = run_test_sample(COMMAND, num_tests)
    print_test_results(test_results)

def run_test_sample(command: str, num_tests: int) -> list[float]:
    test_results = []
    for i in range(num_tests):
        test_results.append(run_shell_timed(command))
    return test_results

def print_test_results(test_results: list[float]) -> None:
    mean = np.mean(test_results)
    left, right = scistats.t.interval(
        confidence=(1 - ALPHA), 
        df=len(test_results), 
        loc=mean,
        scale=scistats.sem(test_results)
    )
    error = (right - left) / 2
    print(f"{round(mean, ROUND_DIGITS)} \u00B1 {round(error, ROUND_DIGITS)} seconds (n={len(test_results)})")

def run_shell_timed(command: str) -> float:
    start = time.time()
    run_shell(command)
    return time.time() - start

def run_shell(command: str) -> None:
    # print(shlex.split(command))
    p = sb.run(shlex.split(command), stdout=sb.DEVNULL)


start_test_runner()

sb.run(shlex.split("rm -f ./a.out"))