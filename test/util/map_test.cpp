#include "../../src/util/map.h"
#include "../testlib/testlib.h"
#include <cstdio>

define_map(int, char);

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
    for(int i = 260; i < 1000; ++i) {
        bool prime = true;
        
    }
}