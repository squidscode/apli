#include "../testlib/testlib.h"
#include "../../../src/util/bitset.h"
#include "../../../src/util/set.h"


int main() {
    _bitset_t *set1 = bitset_new();
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

    _bitset_t *set2 = bitset_new();
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

    set_free(set1);
    set_free(set2);
}