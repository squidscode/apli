#include "../../src/util/map.h"
#include "../testlib/testlib.c"
#include <cstdio>
#include <string.h>
#include <set>

define_map(int, char);
typedef const char* str;
define_map(str, str);
typedef struct _ch_buf_ {
    char buf[50];
} ch_buf;
define_map(ch_buf, int);
define_map(char, int);


size_t ch_buf_eq(ch_buf str1, ch_buf str2) {
    return strcmp(str1.buf, str2.buf) == 0;
}

size_t ch_buf_hash(ch_buf str1) {
    size_t hash = 0;
    char *ptr = (char*) ((void*) &str1.buf);
    size_t i = 0;
    size_t mod = sizeof(size_t) / sizeof(char);
    size_t offset = 0;
    while(ptr[i] != '\0') {
        hash ^= ((255UL & ptr[i++]) << (8 * offset++));
        offset %= mod;
    }
    return hash;
}


size_t str_eq(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
}

size_t str_hash(const char* str) {
    size_t hash = 0;
    char *ptr = (char*) ((void*) &str);
    size_t i = 0;
    size_t mod = sizeof(size_t) / sizeof(char);
    size_t offset = 0;
    while(ptr[i] != '\0') {
        hash ^= ((255UL & ptr[i++]) << (8 * offset++));
        offset %= mod;
    }
    return hash;
}


int main() {
    Map(int, char) *map = map_new(int, char);
    map_insert(map, 10, 'c');
    assertTrue(map_at(map, 10) == 'c');
    assertTrue(map_count(map, 50) == 0);
    assertTrue(map_count(map, 10) == 1);
    map_insert(map, 50, 'b');
    assertTrue(map_at(map, 10) == 'c');
    assertTrue(map_at(map, 50) == 'b');
    assertTrue(map_count(map, 50) == 1);
    assertTrue(map_count(map, 10) == 1);
    assertTrue(map_count(map, 20) == 0);
    assertTrue(map_erase(map, 50) == 1);
    assertTrue(map_erase(map, 50) == 0);
    for(int i = 2; i < 260; ++i) {
        bool prime = true;
        for(int j = 2; j < i; ++j) {
            if(i % j == 0) {prime = false; break;}
        }
        if(prime) map_insert(map, i, 'p');
        else      map_insert(map, i, 'c');
    }
    assertTrue(map_at(map, 77) == 'c');
    assertTrue(map_at(map, 101) == 'p');
    /* Faster Prime Calculation [Only perform mod on primes] */
    for(int i = 260; i < 1500; ++i) {
        bool prime = true;
        for(int j = 2; j < i; ++j) {
            if((map_at(map, j) == 'p') && (i % j == 0)) {prime = false; break;}
        }
        if(prime)   map_insert(map, i, 'p');
        else        map_insert(map, i, 'c');
    }
    assertTrue(map_at(map, 1009) == 'p');
    assertTrue(map_at(map, 1002) == 'c');
    assertTrue(map_count(map, 1500) == 0);
    assertTrue(map_count(map, 1400) == 1);
    map_insert(map, 2, 'c');
    assertTrue(map_count(map, 2) == 1);
    assertTrue(map_at(map, 2) == 'c');
    map_free(map);
    
    Map(ch_buf, int) *str_map = map_new(ch_buf, int);
    assertTrue(map_size(str_map) == 0);
    ch_buf ch1;
    strcpy(ch1.buf, "one");
    map_insert(str_map, ch1, 10);
    assertTrue(map_size(str_map) == 1);
    assertTrue(map_at(str_map, ch1) == 10);
    ch_buf ch2;
    assertTrue(map_count(str_map, ch2) == 0);
    strcpy(ch2.buf, "two");
    assertTrue(map_count(str_map, ch2) == 0);
    strcpy(ch2.buf, "one");
    // NOTE: this test fails because the default equals+hash isn't the same as the string equals definition
    // assertTrue(map_count(str_map, ch2) == 1); 

    assertTrue(ch_buf_eq(ch1, ch2) == 1);
    assertTrue(ch_buf_hash(ch1) == ch_buf_hash(ch2));
    strcpy(ch2.buf, "two");
    assertTrue(ch_buf_eq(ch1, ch2) == 0);

    map_free(str_map);

    // Define a the str_map with the correct hash and key functions
    str_map = map_new(ch_buf, int);
    map_set_hash(str_map, &ch_buf_hash);
    map_set_key_eq(str_map, &ch_buf_eq);
    strcpy(ch1.buf, "one");
    map_insert(str_map, ch1, 10);
    assertTrue(map_count(str_map, ch1) == 1);
    assertTrue(map_at(str_map, ch1) == 10);
    strcpy(ch2.buf, "one");
    map_insert(str_map, ch2, 20);
    assertTrue(map_size(str_map) == 1); // The size of the map does not increase
    assertTrue(map_at(str_map, ch2) == 20);
    strcpy(ch2.buf, "two");
    assertTrue(map_count(str_map, ch2) == 0);
    assertTrue(map_count(str_map, ch1) == 1);
    assertTrue(map_at(str_map, ch1) == 20);
    assertTrue(map_erase(str_map, ch1) == 1);
    assertTrue(map_count(str_map, ch1) == 0);
    assertTrue(map_size(str_map) == 0);
    map_free(str_map);

    Map(str, str) *map3 = map_new(str, str);
    map_set_hash(map3, &str_hash);
    map_set_key_eq(map3, &str_eq);
    map_insert(map3, "A", "1");
    map_insert(map3, "C", "3");
    map_insert(map3, "D", "4");
    assertTrue(strcmp(map_at(map3, "A"), "1") == 0);
    assertTrue(strcmp(map_at(map3, "C"), "3") == 0);
    assertTrue(strcmp(map_at(map3, "D"), "4") == 0);
    assertTrue(map_count(map3, "B") == 0);
    assertTrue(map_size(map3) == 3);
    map_insert(map3, "B", "three");
    assertTrue(map_size(map3) == 4);
    assertTrue(map_count(map3, "B") == 1);
    assertTrue(strcmp(map_at(map3, "B"), "three") == 0);
    map_insert(map3, "B", "3");
    assertTrue(strcmp(map_at(map3, "B"), "3") == 0);
    map_erase(map3, "B");
    assertTrue(map_count(map3, "B") == 0);
    assertTrue(map_size(map3) == 3);
    map_free(map3);

    Map(char, int) *char_to_ascii = map_new(char, int);
    for(char c = 'A'; c <= 'Z'; ++c) {
        map_insert(char_to_ascii, c, c);
    }
    for(char c = 'a'; c <= 'z'; ++c) {
        map_insert(char_to_ascii, c, c);
    }
    auto list_of_matches = map_get_list(char_to_ascii);
    assertTrue(26 * 2 == list_size(list_of_matches)); // check the size of the list
    std::set<char> ch_set;
    auto iter = get_iterator(list_of_matches);
    while(iter != NULL) {
        assertTrue(0 == ch_set.count(iter_val(iter).key));
        ch_set.insert(iter_val(iter).key);
        assertTrue(iter_val(iter).key == iter_val(iter).value);
        iter = iter_next(iter);
    }
    assertTrue(26 * 2 == list_size(list_of_matches));
    assertTrue(26 * 2 == map_size(char_to_ascii));
    list_free(list_of_matches);
    map_free(char_to_ascii);

    teardown_tests();
}