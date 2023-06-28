#include "../testlib/testlib.c"
#include "../../src/lexer/dfa.h"

define_list(char);
define_dfa(int, char);

List(char)* str_list = NULL;
Iterator(char)* str_to_iter(const char*);

int main() {
    // A simple DFA:
    DFA(int, char) *dfa1 = dfa_new(int, char, 0);
    dfa_add_transition(dfa1, 0, 'A', 1); dfa_add_transition(dfa1, 1, 'B', 2);
    dfa_add_transition(dfa1, 0, 'B', 2); dfa_add_transition(dfa1, 2, 'C', 3);
    dfa_add_transition(dfa1, 0, 'C', 3); dfa_add_transition(dfa1, 3, 'D', 4);
    dfa_add_transition(dfa1, 0, 'D', 4); dfa_add_transition(dfa1, 4, 'E', 5);
    dfa_add_transition(dfa1, 0, 'E', 5); dfa_add_transition(dfa1, 5, 'F', 6);
    dfa_add_transition(dfa1, 0, 'F', 6); dfa_add_transition(dfa1, 6, 'G', 7);
    dfa_add_transition(dfa1, 0, 'G', 7); dfa_add_transition(dfa1, 0, 'X', 7);
    dfa_add_accept_state(dfa1, 7);

    char buf[50];
    strcpy(buf, "A");       assertTrue(0 == dfa_run(dfa1, str_to_iter(buf)));
    strcpy(buf, "B");       assertTrue(0 == dfa_run(dfa1, str_to_iter(buf)));
    strcpy(buf, "C");       assertTrue(0 == dfa_run(dfa1, str_to_iter(buf)));
    strcpy(buf, "L");       assertTrue(0 == dfa_run(dfa1, str_to_iter(buf)));
    strcpy(buf, "X");       assertTrue(1 == dfa_run(dfa1, str_to_iter(buf)));
    strcpy(buf, "ABCDEFG"); assertTrue(1 == dfa_run(dfa1, str_to_iter(buf)));
    strcpy(buf, "EFG");     assertTrue(1 == dfa_run(dfa1, str_to_iter(buf)));

    FDFA(int, char) *fdfa = dfa_to_fdfa(dfa1);
    assertTrue(0 == call(fdfa, str_to_iter("abc")));
    assertTrue(1 == call(fdfa, str_to_iter("BCDEFG")));
    assertTrue(1 == call(fdfa, str_to_iter("FG")));
    assertTrue(0 == call(fdfa, str_to_iter("invalid!")));

    fdfa_free(fdfa);

    // Accepts strings that represent numbers.
    DFA(int, char) *number_dfa = dfa_new(int, char, 0);
    dfa_add_transition(number_dfa, 0, '-', 0); dfa_add_transition(number_dfa, 1, '0', 1);
    dfa_add_transition(number_dfa, 0, '1', 1); dfa_add_transition(number_dfa, 1, '1', 1);
    dfa_add_transition(number_dfa, 0, '2', 1); dfa_add_transition(number_dfa, 1, '2', 1);
    dfa_add_transition(number_dfa, 0, '3', 1); dfa_add_transition(number_dfa, 1, '3', 1);
    dfa_add_transition(number_dfa, 0, '4', 1); dfa_add_transition(number_dfa, 1, '4', 1);
    dfa_add_transition(number_dfa, 0, '5', 1); dfa_add_transition(number_dfa, 1, '5', 1);
    dfa_add_transition(number_dfa, 0, '6', 1); dfa_add_transition(number_dfa, 1, '6', 1);
    dfa_add_transition(number_dfa, 0, '7', 1); dfa_add_transition(number_dfa, 1, '7', 1);
    dfa_add_transition(number_dfa, 0, '8', 1); dfa_add_transition(number_dfa, 1, '8', 1);
    dfa_add_transition(number_dfa, 0, '9', 1); dfa_add_transition(number_dfa, 1, '9', 1);
    dfa_add_transition(number_dfa, 0, '0', 2);

    dfa_add_accept_state(number_dfa, 1);
    dfa_add_accept_state(number_dfa, 2);

    assertTrue(0 == dfa_run(number_dfa, str_to_iter("asdf")));
    assertTrue(0 == dfa_run(number_dfa, str_to_iter("-")));
    assertTrue(0 == dfa_run(number_dfa, str_to_iter("-0.0")));
    assertTrue(0 == dfa_run(number_dfa, str_to_iter("01245")));
    assertTrue(0 == dfa_run(number_dfa, str_to_iter("1999A245")));
    assertTrue(0 == dfa_run(number_dfa, str_to_iter("1999A245\n")));
    assertTrue(1 == dfa_run(number_dfa, str_to_iter("0")));
    assertTrue(1 == dfa_run(number_dfa, str_to_iter("12404")));
    assertTrue(1 == dfa_run(number_dfa, str_to_iter("-0")));
    assertTrue(1 == dfa_run(number_dfa, str_to_iter("12345678909876453412")));
    assertTrue(1 == dfa_run(number_dfa, str_to_iter("-46456")));

    assertTrue(0 == dfa_run(number_dfa, str_to_iter(" 123")));
    dfa_add_transition(number_dfa, 0, ' ', 0);
    assertTrue(1 == dfa_run(number_dfa, str_to_iter(" 123")));

    assertTrue(1 == dfa_run(number_dfa, str_to_iter(" 123")));
    dfa_remove_transition(number_dfa, 0, ' ');
    assertTrue(0 == dfa_run(number_dfa, str_to_iter(" 123")));

    assertTrue(0 == dfa_run(number_dfa, str_to_iter("")));
    dfa_add_accept_state(number_dfa, 0);
    assertTrue(1 == dfa_run(number_dfa, str_to_iter("")));

    assertTrue(1 == dfa_run(number_dfa, str_to_iter("")));
    dfa_remove_accept_state(number_dfa, 0);
    assertTrue(0 == dfa_run(number_dfa, str_to_iter("")));

    
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
    return get_iterator(str_list);
}