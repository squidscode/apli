#include "../../src/util/map.h"
#include "../testlib/testlib.h"
#include <cstdio>
#include <string.h>

define_map(int, char);

typedef struct _ch_buf_ {
    char buf[50];
} ch_buf;

define_map(ch_buf, int);

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



int main() {
    Map(int, char) *map = new_map(int, char);
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
    free_map(map);
    
    Map(ch_buf, int) *str_map = new_map(ch_buf, int);
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

    free_map(str_map);

    // Define a the str_map with the correct hash and key functions
    str_map = new_map(ch_buf, int);
    set_map_hash(str_map, &ch_buf_hash);
    set_map_key_eq(str_map, &ch_buf_eq);
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
}