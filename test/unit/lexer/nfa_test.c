#include "../../../src/lexer/nfa.h"
#include "../testlib/testlib.h"

define_list(size_t);
define_list(char);
define_list(int);
define_set(size_t);
init_dfa_types(size_t, char);
define_dfa(size_t, char);
init_nfa(int, char);
define_nfa(int, char);

List(char)* str_list = NULL;
Iterator(char)* str_to_iter(const char*);

int main() {
    Nfa(int, char) *nfa1 = nfa_new(int, char, 0);
    nfa_add_accept_state(nfa1, 0);
    nfa_add_transition(nfa1, 0, '1', 1);
    nfa_add_transition(nfa1, 1, '0', 0);
    nfa_add_alphabet_transition(nfa1, 1, 1);
    Set(char) *alphabet = set_new(char);
    for(int i = 0; i < 126; ++i) {
        set_insert(alphabet, i);
    }
    
    __auto_type *dfa = nfa_to_dfa(nfa1, alphabet);
    assertTrue(1 == dfa_run(dfa, str_to_iter("1ewooeowp0110")));
    dfa_free(dfa);
    nfa_free(nfa1);

    // The next to examples are from:
    // https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton

    set_free(alphabet);
    alphabet = set_new(char);
    set_insert(alphabet, '0');
    set_insert(alphabet, '1');

    // This nfa determines if the string ends in 1. The alphabet is {0, 1}
    Nfa(int, char) *nfa_example_1 = nfa_new(int, char, 0);
    nfa_add_accept_state(nfa_example_1, 1);
    nfa_add_alphabet_transition(nfa_example_1, 0, 0);
    nfa_add_transition(nfa_example_1, 0, '1', 1);
    dfa = nfa_to_dfa(nfa_example_1, alphabet);
    assertTrue(0 == dfa_run(dfa, str_to_iter("")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("0010")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("0")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("0a")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("0a1")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("1")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("010001")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("11101")));
    dfa_free(dfa);
    nfa_free(nfa_example_1);

    // This nfa determines if the string contains an even number of 1s or 0s.
    Nfa(int, char) *nfa_example_2 = nfa_new(int, char, 0);
    nfa_add_epsilon_transition(nfa_example_2, 0, 1);
    nfa_add_transition(nfa_example_2, 1, '0', 2);
    nfa_add_transition(nfa_example_2, 1, '1', 1);
    nfa_add_transition(nfa_example_2, 2, '0', 1);
    nfa_add_transition(nfa_example_2, 2, '1', 2);
    nfa_add_epsilon_transition(nfa_example_2, 0, 3);
    nfa_add_transition(nfa_example_2, 3, '0', 3);
    nfa_add_transition(nfa_example_2, 3, '1', 4);
    nfa_add_transition(nfa_example_2, 4, '0', 4);
    nfa_add_transition(nfa_example_2, 4, '1', 3);
    nfa_add_accept_state(nfa_example_2, 1);
    nfa_add_accept_state(nfa_example_2, 3);
    dfa = nfa_to_dfa(nfa_example_2, alphabet);

    assertTrue(0 == dfa_run(dfa, str_to_iter("a")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("0e")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("0001")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("01")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("10")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("101010")));
    assertTrue(0 == dfa_run(dfa, str_to_iter("01000101")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("111")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("000")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("1")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("0")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("0111001")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("0011")));
    assertTrue(1 == dfa_run(dfa, str_to_iter("0000001")));

    set_free(alphabet);
    dfa_free(dfa);
    nfa_free(nfa_example_2);

    list_free(str_list);

    teardown_tests();
}

Iterator(char)* str_to_iter(const char* str) {
    if(str_list == NULL) str_list = list_new(char);
    else {
        list_free(str_list);
        str_list = list_new(char);
    }
    size_t size = strlen(str);
    for(size_t i = 0; i < size; ++i) {
        list_push_back(str_list, str[i]);
    }
    return list_get_iterator(str_list);
}

const char* construct_int_char_transition_string(Set(int) *from, char transition, Set(int) *to) {
    char *buf = (char*) malloc(50 * sizeof(char));
    List(int) *from_l = set_get_list(from);
    List(int) *to_l = set_get_list(to);
    sprintf(buf, "{");
    while(0 < list_size(from_l)) {
        sprintf(buf, "%s%i, ", buf, list_get_first(from_l));
        list_pop_front(from_l);
    }
    sprintf(buf, "%s}", buf);

    sprintf(buf, "%s >> %c >> ", buf, transition);

    sprintf(buf, "%s{", buf);
    while(0 < list_size(to_l)) {
        sprintf(buf, "%s%i, ", buf, list_get_first(to_l));
        list_pop_front(to_l);
    }
    sprintf(buf, "%s}", buf);

    return buf;
}
