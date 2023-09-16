#include "../testlib/testlib.h"
#include "../../../src/lexer/flat_dfa.c"


List(char)* str_list = NULL;
Iterator(char)* str_to_iter(const char*);

int main() {
    // A simple DFA:
    _flat_dfa_t *dfa1 = flat_dfa_new(8);
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
    strcpy(buf, "ABCDEFG");     assertTrue(1 == dfa_run(dfa1, str_to_iter(buf)));
    strcpy(buf, "EFG");         assertTrue(1 == dfa_run(dfa1, str_to_iter(buf)));
    strcpy(buf, "EFG asdf");    assertTrue(3 == dfa_run_greedy_iter(dfa1, str_to_iter(buf)));
    strcpy(buf, "X EFG asdf");  assertTrue(1 == dfa_run_greedy_iter(dfa1, str_to_iter(buf)));


    dfa_free(dfa1);

    // Accepts strings that represent numbers.
    Dfa(size_t, char) *num_dfa = dfa_new(size_t, char, 0);
    dfa_add_transition(num_dfa, 0, '-', 0); dfa_add_transition(num_dfa, 1, '0', 1);
    dfa_add_transition(num_dfa, 0, '1', 1); dfa_add_transition(num_dfa, 1, '1', 1);
    dfa_add_transition(num_dfa, 0, '2', 1); dfa_add_transition(num_dfa, 1, '2', 1);
    dfa_add_transition(num_dfa, 0, '3', 1); dfa_add_transition(num_dfa, 1, '3', 1);
    dfa_add_transition(num_dfa, 0, '4', 1); dfa_add_transition(num_dfa, 1, '4', 1);
    dfa_add_transition(num_dfa, 0, '5', 1); dfa_add_transition(num_dfa, 1, '5', 1);
    dfa_add_transition(num_dfa, 0, '6', 1); dfa_add_transition(num_dfa, 1, '6', 1);
    dfa_add_transition(num_dfa, 0, '7', 1); dfa_add_transition(num_dfa, 1, '7', 1);
    dfa_add_transition(num_dfa, 0, '8', 1); dfa_add_transition(num_dfa, 1, '8', 1);
    dfa_add_transition(num_dfa, 0, '9', 1); dfa_add_transition(num_dfa, 1, '9', 1);
    dfa_add_transition(num_dfa, 0, '0', 2);

    dfa_add_accept_state(num_dfa, 1);
    dfa_add_accept_state(num_dfa, 2);

    _flat_dfa_t *number_dfa = flat_dfa_from_compressed_dfa(num_dfa);

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
    
    const char* serialized_dfa = flat_dfa_serialize(number_dfa);
    size_t sz = ((const size_t *)((void*) serialized_dfa))[0];
    size_t num_transitions = ((const size_t *)((void*) serialized_dfa))[1];
    printf("num states: %zu, sz: %zu\n", num_transitions, sz);
    size_t last = (num_transitions << sz) + (sizeof(size_t) << 1);
    for(size_t i = 0; i < last; ++i) {
        if(i % 16 == 0)
            printf("\n");
        printf("'\\x");
        printf("%02hhX", serialized_dfa[i]);
        printf("'");
        if(i != last - 1)
            printf(", ");
    }
    printf("\n");

    dfa_free(number_dfa);

    number_dfa = flat_dfa_deserialize(serialized_dfa);
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
    dfa_free(number_dfa);

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