#include "../../src/lexer/regex.h"
#include "../../src/lexer/regex.c"
#include "../testlib/testlib.c"

int main() {
    Regex *reg1 = regex_from("abc");
    regex_compile(reg1);
    assertTrue(0 == regex_run(reg1, "hello world"));
    assertTrue(1 == regex_run(reg1, "abc"));
    regex_free(reg1);

    Regex *regex2 = regex_from("[dlb]og");
    regex_compile(regex2);
    assertTrue(0 == regex_run(regex2, "hello world"));
    assertTrue(1 == regex_run(regex2, "dog"));
    assertTrue(1 == regex_run(regex2, "log"));
    assertTrue(1 == regex_run(regex2, "bog"));
    assertTrue(0 == regex_run(regex2, "fog"));
    regex_free(regex2);

    Regex *regex3 = regex_from("dog|log|bog");
    regex_compile(regex3);
    assertTrue(0 == regex_run(regex3, "hello world"));
    assertTrue(1 == regex_run(regex3, "dog"));
    assertTrue(1 == regex_run(regex3, "log"));
    assertTrue(1 == regex_run(regex3, "bog"));
    assertTrue(0 == regex_run(regex3, "fog"));
    regex_free(regex3);

    Regex *regex4 = regex_from("[df]og|log|bog");
    regex_compile(regex4);
    assertTrue(0 == regex_run(regex4, "hello world"));
    assertTrue(1 == regex_run(regex4, "dog"));
    assertTrue(1 == regex_run(regex4, "log"));
    assertTrue(1 == regex_run(regex4, "bog"));
    assertTrue(1 == regex_run(regex4, "fog"));
    assertTrue(0 == regex_run(regex4, "cog"));
    regex_free(regex4);

    Regex *regex5 = regex_from("([df]og)?[ ]*|log|bog");
    regex_compile(regex5);
    assertTrue(0 == regex_run(regex5, "hello world"));
    assertTrue(1 == regex_run(regex5, ""));
    assertTrue(1 == regex_run(regex5, "     "));
    assertTrue(1 == regex_run(regex5, "dog"));
    assertTrue(1 == regex_run(regex5, "fog"));
    assertTrue(1 == regex_run(regex5, "dog "));
    assertTrue(1 == regex_run(regex5, "fog "));
    assertTrue(1 == regex_run(regex5, "dog  "));
    assertTrue(1 == regex_run(regex5, "fog  "));
    assertTrue(1 == regex_run(regex5, "dog   "));
    assertTrue(1 == regex_run(regex5, "fog   "));
    assertTrue(1 == regex_run(regex5, "log"));
    assertTrue(1 == regex_run(regex5, "bog"));
    assertTrue(0 == regex_run(regex5, "cog"));
    regex_free(regex5);

    Regex *regex6 = regex_from("([df]og)*[ ]*|(lo)+g|(b(_*)o)g");
    regex_compile(regex6);
    assertTrue(0 == regex_run(regex6, "hello world"));
    assertTrue(1 == regex_run(regex6, ""));
    assertTrue(1 == regex_run(regex6, "     "));
    assertTrue(1 == regex_run(regex6, "dog"));
    assertTrue(1 == regex_run(regex6, "dogdog"));
    assertTrue(1 == regex_run(regex6, "dogfog"));
    assertTrue(1 == regex_run(regex6, "dogfogdogdogfogfog"));
    assertTrue(1 == regex_run(regex6, "fog"));
    assertTrue(1 == regex_run(regex6, "dog "));
    assertTrue(1 == regex_run(regex6, "fog "));
    assertTrue(1 == regex_run(regex6, "dog  "));
    assertTrue(1 == regex_run(regex6, "fog  "));
    assertTrue(1 == regex_run(regex6, "dog   "));
    assertTrue(1 == regex_run(regex6, "fog   "));
    assertTrue(1 == regex_run(regex6, "log"));
    assertTrue(1 == regex_run(regex6, "lolololog"));
    assertTrue(0 == regex_run(regex6, "g"));
    assertTrue(1 == regex_run(regex6, "bog"));
    assertTrue(1 == regex_run(regex6, "b_og"));
    assertTrue(1 == regex_run(regex6, "b____og"));
    assertTrue(0 == regex_run(regex6, "cog"));
    regex_free(regex6);
}

// TODO remove this code:
const char* construct_int_char_transition_string(Set(size_t) *from, char transition, Set(size_t) *to) {
    char *buf = (char*) malloc(1000 * sizeof(char));
    List(size_t) *from_l = set_get_list(from);
    List(size_t) *to_l = set_get_list(to);
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
    dprintf("%s\n", buf);
    return buf;
}