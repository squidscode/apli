#include "../../../src/util/list.h"
#include <list>
#include "../testlib/testlib.h"

define_list(int);

/*
#define List(TYPE)                  TYPE##_list_t
#define new_list(TYPE)              (_new_##TYPE##_list())
#define list_size(list)             (list->_size)
#define list_get_first(list)        (list->_fns->_get_first((list)))
#define list_get_last(list)         (list->_fns->_get_last((list)))
#define list_push_front(list, val)  (list->_fns->_push_front((list), (val)))
#define list_push_back(list, val)   (list->_fns->_push_back((list), (val)))
#define list_pop_front(list)        (list->_fns->_pop_front((list)))
#define list_pop_back(list)         (list->_fns->_pop_back((list)))
#define free_list(list)             (list->_fns->_free((list)))
*/

int main() {
    List(int) *int_list = list_new(int);
    assertTrue(list_size(int_list) == 0);
    list_push_back(int_list, 1);
    assertTrue(list_size(int_list) == 1);
    list_push_back(int_list, 2);
    list_push_back(int_list, 3);
    list_push_back(int_list, 4);
    assertTrue(list_size(int_list) == 4);
    assertTrue(list_get_first(int_list) == 1);
    assertTrue(list_get_last(int_list) == 4);
    list_push_front(int_list, 0);
    assertTrue(list_size(int_list) == 5);
    assertTrue(list_get_first(int_list) == 0);
    list_pop_front(int_list);
    assertTrue(list_size(int_list) == 4);
    assertTrue(list_get_first(int_list) == 1);
    list_pop_back(int_list); // 1 2 3
    assertTrue(list_size(int_list) == 3);
    assertTrue(list_get_last(int_list) == 3);
    
    int nxt = 3;
    while(list_size(int_list) > 0) {
        assertTrue(list_get_last(int_list) == nxt--);
        list_pop_back(int_list);
    }

    while(nxt < 100) {
        list_push_front(int_list, nxt++);
    }
    assertTrue(list_size(int_list) == 100);
    assertTrue(list_get_last(int_list) == 0);
    assertTrue(list_get_first(int_list) == 99);

    for(int i = 0; i < 100000; ++i) {
        list_push_back(int_list, i);
    }
    assertTrue(list_size(int_list) == 100100);
    assertTrue(list_get_last(int_list) == 99999);

    // no assertions can be made for free without creating a mock.
    list_free(int_list);

    // brute force testing to look for memory leaks:
    std::list<int> list_mirror;
    for(int test = 0; test < 10000; ++test) { // 1000 tests
        int_list = list_new(int);
        for(int i = 0; i < 1000; ++i) {
            int val = rand();
            int st = rand() % 4;
            if(st == 0) {
                list_mirror.push_back(val);
                list_push_back(int_list, val);
            } else if(st == 1) {
                list_mirror.push_front(val);
                list_push_front(int_list, val);
            } else if(st == 2) {
                if(list_mirror.size() == 0) continue;
                list_mirror.pop_front();
                list_pop_front(int_list);
            } else if(st == 3) {
                if(list_mirror.size() == 0) continue;
                list_mirror.pop_back();
                list_pop_back(int_list);
            }
        }
        bool passed = true;
        while(list_mirror.size() > 0) {
            assert(list_mirror.front() == list_get_first(int_list));
            passed = passed && (list_mirror.front() == list_get_first(int_list));
            list_mirror.pop_front();
            list_pop_front(int_list);
        }
        assert(list_mirror.size() == list_size(int_list));
        passed = passed && (list_mirror.size() == list_size(int_list));
        assertTrueQuiet(passed);
        
        list_mirror.clear();
        list_free(int_list);
    }

    teardown_tests();
}