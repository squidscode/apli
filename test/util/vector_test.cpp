#include "../../src/util/vector.c"
#include "../testlib/testlib.c"
#include <cstdio>
#include <vector>

#define NUM_TESTS       10000
#define NUM_OPERATIONS  1000

#define printv(sz, ind_getter) \
    printf("{"); \
    for(size_t ind = 0; ind < sz; ++ind) { \
        printf("%d, ", (ind_getter)); \
    } \
    printf("} ")
#define dbif(expr) \
    if(!(expr)) { \
        printv(vector_size(v), vector_get(v, ind)); \
        printv(v_test.size(), v_test[ind]); \
        printf("\n"); \
    }

/*
#define Vector(TYPE)                    _##TYPE##_vector_t
#define new_vector(TYPE)                (_new_##TYPE##_vector())
#define vector_get(list, index)         ((list)->_fns->get((list), (index)))
#define vector_set(list, index, val)    ((list)->_fns->set((list), (index), (val)))
#define vector_remove(list, index)      ((list)->_fns->remove((list), (index)))
#define vector_pop_right(list)   ((list)->_fns->pop_right((list)))
#define vector_push_right(list, val)    ((list)->_fns->push_right((list), (val)))
#define vector_resize(list, size)       ((list)->_fns->resize((list), (size)))             
#define vector_size(list)               ((list)->_fns->size((list)))
*/

define_vector(int);

int main() {
    Vector(int)* vi = new_vector(int);
    assertTrue(vector_size(vi) == 0);
    vector_push_back(vi, 5);
    assertTrue(vector_size(vi) == 1);
    assertTrue(vector_get(vi, 0) == 5);
    vector_push_back(vi, 6);
    vector_push_back(vi, 7);
    vector_push_back(vi, 8);
    assertTrue(vector_get(vi, 3) == 8);
    vector_pop_back(vi);
    assertTrue(vector_size(vi) == 3);
    assertTrue(vector_get(vi, 2) == 7);
    vector_resize(vi, 100);
    assertTrue(vector_size(vi) == 100); // Nothing should change
    vector_remove(vi, 1);
    assertTrue(vector_get(vi, 0) == 5);
    assertTrue(vector_get(vi, 1) == 7);
    assertTrue(vector_size(vi) == 99);
    vector_set(vi, 10, 1);
    assertTrue(vector_get(vi, 0) == 5);
    assertTrue(vector_get(vi, 1) == 7);
    assertTrue(vector_get(vi, 10) == 1);
    assertTrue(vector_size(vi) == 99);
    vector_set(vi, 5, 20);
    assertTrue(vector_get(vi, 5) == 20);
    assertTrue(vector_size(vi) == 99);
    vector_resize(vi, 2);
    assertTrue(vector_size(vi) == 2);
    assertTrue(vector_get(vi, 0) == 5);
    assertTrue(vector_get(vi, 1) == 7);
    // exit(0);

    Vector(int) *v = new_vector(int);
    std::vector<int> v_test;
    for(int i = 0; i < NUM_TESTS; ++i) {
        std::vector<int> pp_vec;
        std::vector<int> prev_vector;
        for(int j = 0; j < NUM_OPERATIONS; ++j) {
            int r = rand();
            int mod = 7;
            int c = r % mod;
            pp_vec = std::vector<int>(prev_vector);
            prev_vector = std::vector<int>(v_test);
            if (c == 0) {
                if(v_test.size() == 0) continue;
                // printf("GET\n");
                size_t index = (r % v_test.size());
                assertTrueQuiet(vector_get(v, index) == v_test[index]);
            } else if (c == 1) {
                if(v_test.size() == 0) continue;
                size_t index = (r % v_test.size());
                // printf("SET\n");
                assertTrueQuiet(vector_get(v,index) == v_test[index]);
                assertTrueQuiet(vector_size(v) == v_test.size());
                int randVal = rand();
                vector_set(v, index, randVal); v_test[index] = randVal;
                assertTrueQuiet(vector_get(v,index) == v_test[index]);
                assertTrueQuiet(vector_size(v) == v_test.size());
            } else if (c == 2) {
                if(v_test.size() == 0) continue;
                // printf("POP BACK\n");
                assertTrueQuiet(vector_get(v, v_test.size() - 1) == v_test[v_test.size() - 1]);
                assertTrueQuiet(vector_size(v) == v_test.size());
                size_t prev_size = v_test.size();
                vector_pop_back(v); v_test.pop_back();
                assertTrueQuiet(vector_size(v) == v_test.size());
                // dbif(vector_size(v) == v_test.size());
                assertTrueQuiet(vector_size(v) == (prev_size - 1));
            } else if (c == 3 || c == 6) {
                // printf("PUSH BACK\n");
                assertTrueQuiet(vector_size(v) == v_test.size());
                // dbif(vector_size(v) == v_test.size());
                int randVal = rand();
                if(v_test.size() >= 30) continue; // keep the size of the array small :)
                vector_push_back(v, randVal); v_test.push_back(randVal);
                assertTrueQuiet(vector_size(v) == v_test.size());
                assertTrueQuiet(vector_get(v, v_test.size() - 1) == v_test[v_test.size() - 1]);
            } else if (c == 4) { // Nothing here should change
                // printf("RESIZE\n");
                int randVal = (rand() % 15 - 7); // only increase the size by at most 15
                randVal = ((((int) v_test.size() + randVal) >= 0) ? (v_test.size() + randVal) : 0);
                // printf("res: %d\n", randVal);
                assertTrueQuiet(vector_size(v) == v_test.size());
                vector_resize(v, randVal); v_test.resize(randVal);
                assertTrueQuiet(vector_size(v) == v_test.size());
            } else if (c == 5) {
                // printf("SIZE\n");
                assertTrueQuiet(vector_size(v) == v_test.size());
                // dbif(vector_size(v) == v_test.size());
            }
            int pass = 1;
            for(int ind = 0; ind < v_test.size(); ++ind) {
                assertTrueQuiet((pass = (pass && (vector_get(v, ind) == v_test[ind]))));
                dbif(vector_get(v, ind) == v_test[ind]);
            }
            if(pass == 0) {
                printf("2nd prev: "); printv(pp_vec.size(), pp_vec[ind]);
                printf("1st prev: "); printv(prev_vector.size(), prev_vector[ind]); 
                printf("c:%d\n",c); 
                exit(0);
            };
        }
        assertTrueQuiet(vector_size(v) == v_test.size());
        // printf("v_test size : %lu\n", v_test.size());
        for(int ind = 0; ind < v_test.size(); ++ind) {
            assertTrueQuiet(vector_get(v, ind) == v_test[ind]);
            dbif(vector_get(v, ind) == v_test[ind]);
        }
        vector_free(v);
        v = new_vector(int); v_test.clear();
    }
}