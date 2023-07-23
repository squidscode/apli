#include "../testlib/testlib.h"
#include "../../src/util/set.h"

define_list(int);
define_set(int);

int main() {
    Set(int) *set1 = set_new(int);
    set_insert(set1, 5);
    assertTrue(0 == set_count(set1, 0));
    assertTrue(1 == set_count(set1, 5));
    assertTrue(1 == set_size(set1));
    assertTrue(0 == set_erase(set1, 1));
    assertTrue(1 == set_erase(set1, 5));
    set_insert(set1, 2);
    set_insert(set1, 3);
    set_insert(set1, 3);
    set_insert(set1, 5);
    set_insert(set1, 7);
    set_insert(set1, 11);
    assertTrue(5 == set_size(set1));

    Set(int) *set2 = set_new(int);
    set_insert(set2, 1);
    assertTrue(0 == set_equals(set1, set2));
    set_erase(set2, 1);
    set_insert(set2, 2);
    set_insert(set2, 3);
    set_insert(set2, 5);
    set_insert(set2, 7);
    assertTrue(0 == set_equals(set1, set2));
    set_insert(set2, 11);
    assertTrue(1 == set_equals(set1, set2));
    set_insert(set2, 13);
    assertTrue(0 == set_equals(set1, set2));

    List(int) *set1_list = set_get_list(set1);
    Iterator(int) *iter = list_get_iterator(set1_list);
    while(iter != NULL) {
        assertTrue(set_count(set2, iter_val(iter))); // all values in set1 are in set2!
        iter = iter_next(iter);
    }
    list_free(set1_list);

    set_free(set1);
    set_free(set2);

    teardown_tests();
}