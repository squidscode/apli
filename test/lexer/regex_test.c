#include "../../src/lexer/regex.h"
#include "../../src/lexer/regex.c"
#include "../testlib/testlib.c"
#include <stdio.h>

void print_matches(const char *str, List(_regex_match_t) *list_of_matches);
const char* ftoca(const char* file_path);

int main() {
    Regex *reg1 = regex_from("abc");
    regex_compile(reg1);
    assertTrue(0 == regex_run(reg1, "hello world"));
    assertTrue(1 == regex_run(reg1, "abc"));
    const char *str = "here's abc a long abc string with many matches... abc... abdsabcdc";
    List(_regex_match_t) *list_of_matches = regex_find_all(reg1, str);
    print_matches(str, list_of_matches);
    list_free(list_of_matches);
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

    Regex *regex5 = regex_from("([df]og)?[ ]+|log|bog");
    regex_compile(regex5);
    assertTrue(0 == regex_run(regex5, "helloworld"));
    assertTrue(0 == regex_run(regex5, ""));
    assertTrue(1 == regex_run(regex5, "     "));
    assertTrue(1 == regex_run(regex5, "dog "));
    assertTrue(1 == regex_run(regex5, "fog "));
    assertTrue(1 == regex_run(regex5, "dog "));
    assertTrue(1 == regex_run(regex5, "fog "));
    assertTrue(1 == regex_run(regex5, "dog  "));
    assertTrue(1 == regex_run(regex5, "fog  "));
    assertTrue(1 == regex_run(regex5, "dog   "));
    assertTrue(1 == regex_run(regex5, "fog   "));
    assertTrue(1 == regex_run(regex5, "log"));
    assertTrue(1 == regex_run(regex5, "bog"));
    assertTrue(1 == regex_run(regex5, "asdffewf  bog fdjk\n\nkwefh"));
    str = "asdffewf  bog fdjk\n\nkw   effog  hbogfasdfdf   sdfsfddsfsd\n";
    list_of_matches = regex_find_all(regex5, str);
    print_matches(str, list_of_matches);
    list_free(list_of_matches);
    assertTrue(1 == regex_run(regex5, "  bog fdjk\n\nkwefh"));
    assertTrue(0 == regex_run(regex5, "cog"));
    regex_free(regex5);

    Regex *regex6 = regex_from("([df]og)+|([df]og)*[ ]+");
    regex_compile(regex6);
    assertTrue(0 == regex_run(regex6, "helloworld"));
    assertTrue(0 == regex_run(regex6, ""));
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
    regex_free(regex6);
    regex6 = regex_from("(lo)+g");
    regex_compile(regex6);
    assertTrue(1 == regex_run(regex6, "log"));
    assertTrue(1 == regex_run(regex6, "lolololog"));
    assertTrue(0 == regex_run(regex6, "g"));
    regex_free(regex6);
    regex6 = regex_from("(b(_+)o)g");
    regex_compile(regex6);
    assertTrue(0 == regex_run(regex6, "bog"));
    assertTrue(1 == regex_run(regex6, "b_og"));
    assertTrue(1 == regex_run(regex6, "b____og"));
    assertTrue(0 == regex_run(regex6, "b.....og"));
    assertTrue(0 == regex_run(regex6, "b.._...og"));
    assertTrue(0 == regex_run(regex6, "b____o_g"));
    assertTrue(0 == regex_run(regex6, "cog"));
    regex_free(regex6);

    Regex *negated_regex = regex_from("[^abcd]");
    regex_compile(negated_regex);
    assertTrue(0 == regex_run(negated_regex, "a"));
    assertTrue(0 == regex_run(negated_regex, "b"));
    assertTrue(0 == regex_run(negated_regex, "c"));
    assertTrue(0 == regex_run(negated_regex, "d"));
    assertTrue(1 == regex_run(negated_regex, "e"));
    assertTrue(1 == regex_run(negated_regex, "x"));
    assertTrue(1 == regex_run(negated_regex, "\n"));
    regex_free(negated_regex);

    Regex *repeat_regex = regex_from("^a{1,2}$");
    regex_compile(repeat_regex);
    assertTrue(0 == regex_run(repeat_regex, ""));
    assertTrue(1 == regex_run(repeat_regex, "a"));
    assertTrue(1 == regex_run(repeat_regex, "aa"));
    assertTrue(0 == regex_run(repeat_regex, "aaa"));
    assertTrue(0 == regex_run(repeat_regex, "aaaaa"));
    assertTrue(0 == regex_run(repeat_regex, "b"));
    regex_free(repeat_regex);

    repeat_regex = regex_from("^a{1}$");
    regex_compile(repeat_regex);
    assertTrue(0 == regex_run(repeat_regex, ""));
    assertTrue(1 == regex_run(repeat_regex, "a"));
    assertTrue(0 == regex_run(repeat_regex, "aa"));
    assertTrue(0 == regex_run(repeat_regex, "aaa"));
    assertTrue(0 == regex_run(repeat_regex, "aaaaa"));
    assertTrue(0 == regex_run(repeat_regex, "b"));
    regex_free(repeat_regex);

    repeat_regex = regex_from("^a{3,}$");
    regex_compile(repeat_regex);
    assertTrue(0 == regex_run(repeat_regex, ""));
    assertTrue(0 == regex_run(repeat_regex, "a"));
    assertTrue(0 == regex_run(repeat_regex, "aa"));
    assertTrue(1 == regex_run(repeat_regex, "aaa"));
    assertTrue(1 == regex_run(repeat_regex, "aaaaa"));
    assertTrue(1 == regex_run(repeat_regex, "aaaaaaaa"));
    assertTrue(0 == regex_run(repeat_regex, "b"));
    regex_free(repeat_regex);

    repeat_regex = regex_from("^a{3,5}$");
    regex_compile(repeat_regex);
    assertTrue(0 == regex_run(repeat_regex, ""));
    assertTrue(0 == regex_run(repeat_regex, "a"));
    assertTrue(0 == regex_run(repeat_regex, "aa"));
    assertTrue(1 == regex_run(repeat_regex, "aaa"));
    assertTrue(1 == regex_run(repeat_regex, "aaaa"));
    assertTrue(1 == regex_run(repeat_regex, "aaaaa"));
    assertTrue(0 == regex_run(repeat_regex, "aaaaaa"));
    assertTrue(0 == regex_run(repeat_regex, "aaaaaaaa"));
    assertTrue(0 == regex_run(repeat_regex, "b"));
    regex_free(repeat_regex);

    repeat_regex = regex_from("^(abcd){3,5}$");
    regex_compile(repeat_regex);
    assertTrue(0 == regex_run(repeat_regex, ""));
    assertTrue(0 == regex_run(repeat_regex, "abcd"));
    assertTrue(0 == regex_run(repeat_regex, "abcdabcd"));
    assertTrue(0 == regex_run(repeat_regex, "aaaaabcdabcdabcd"));
    assertTrue(0 == regex_run(repeat_regex, "abcdabcdabcdaaaa"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcdabcd"));
    assertTrue(0 == regex_run(repeat_regex, "abcdabcdabcdabcdabcdabcd"));
    assertTrue(0 == regex_run(repeat_regex, "abcdabcdabcdabcdabcdabcdabcd"));
    assertTrue(0 == regex_run(repeat_regex, "efwkknfej"));
    regex_free(repeat_regex);

    repeat_regex = regex_from("(abcd){3,5}$");
    regex_compile(repeat_regex);
    assertTrue(0 == regex_run(repeat_regex, ""));
    assertTrue(0 == regex_run(repeat_regex, "abcd"));
    assertTrue(0 == regex_run(repeat_regex, "abcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "aaaaabcdabcdabcd"));
    assertTrue(0 == regex_run(repeat_regex, "abcdabcdabcdaaaa"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcdabcdabcdabcd"));
    assertTrue(0 == regex_run(repeat_regex, "efwkknfej"));
    regex_free(repeat_regex);

    repeat_regex = regex_from("^(abcd){3,5}");
    regex_compile(repeat_regex);
    assertTrue(0 == regex_run(repeat_regex, ""));
    assertTrue(0 == regex_run(repeat_regex, "abcd"));
    assertTrue(0 == regex_run(repeat_regex, "abcdabcd"));
    assertTrue(0 == regex_run(repeat_regex, "aaaaabcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdaaaa"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcdabcdabcd"));
    assertTrue(1 == regex_run(repeat_regex, "abcdabcdabcdabcdabcdabcdabcd"));
    assertTrue(0 == regex_run(repeat_regex, "efwkknfej"));
    regex_free(repeat_regex);

    // FOR FUN!
    const char *contents = ftoca("./src/lexer/regex.c");
    Regex *string_regex = regex_from("[^\\]\"[^\n]*[^\\]\"");
    regex_compile(string_regex);
    list_of_matches = regex_find_all(string_regex, contents);
    print_matches(contents, list_of_matches);
    list_free(list_of_matches);
    regex_free(string_regex);

    Regex *type_regex = regex_from("size_t |void |char |int ");
    regex_compile(type_regex);
    list_of_matches = regex_find_all(type_regex, contents);
    print_matches(contents, list_of_matches);
    list_free(list_of_matches);
    regex_free(type_regex);

    Regex *for_regex = regex_from("for");
    Regex *open_paren = regex_from("\\(");
    Regex *closed_paren = regex_from("\\)");

    regex_compile(for_regex);
    regex_compile(open_paren);
    regex_compile(closed_paren);

    list_of_matches = regex_find_all(for_regex, contents);
    print_matches(contents, list_of_matches);
    list_free(list_of_matches);

    list_of_matches = regex_find_all(open_paren, contents);
    print_matches(contents, list_of_matches);
    list_free(list_of_matches);

    list_of_matches = regex_find_all(closed_paren, contents);
    print_matches(contents, list_of_matches);
    list_free(list_of_matches);

    regex_free(for_regex);
    regex_free(open_paren);
    regex_free(closed_paren);
}

void print_matches(const char *str, List(_regex_match_t) *list_of_matches) {
    printf("\n" BLUE "--- REGEX MATCH RESULT ---" RESET "\n");
    // printf(GREEN "%s" RESET "\n", str);
    size_t ind = 0;
    size_t str_size = strlen(str);
    while(list_size(list_of_matches)) {
        _regex_match_t next_match = list_get_front(list_of_matches); list_pop_front(list_of_matches);
        while(ind < next_match.begin)
            printf("%c", str[ind++]);
        printf(RED);
        // printf("{{{%zu, %zu}}}", next_match.begin, next_match.length);
        while(ind < next_match.begin + next_match.length)
            printf("%c", str[ind++]);
        printf(RESET);
    }
    while(ind < str_size)
        printf("%c", str[ind++]);
    printf("\n\n");
}

const char* ftoca(const char* file_path) {
    FILE *fp;

    fp = fopen(file_path, "r");
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);

    char *buff = (char*) malloc(sz+1);

    fseek(fp, 0L, SEEK_SET);
    for(size_t i = 0; i < sz; ++i) {
        buff[i] = fgetc(fp);
    }
    buff[sz] = '\0';
    return buff;
}

// TODO remove this code:
const char* construct_int_char_transition_string(Set(size_t) *from, char transition, Set(size_t) *to) {
    char *buf = (char*) malloc(1000 * sizeof(char));
    List(size_t) *from_l = set_get_list(from);
    List(size_t) *to_l = set_get_list(to);
    sprintf(buf, "{");
    while(0 < list_size(from_l)) {
        sprintf(buf, "%s%zu, ", buf, list_get_first(from_l));
        list_pop_front(from_l);
    }
    sprintf(buf, "%s}", buf);

    sprintf(buf, "%s >> %c >> ", buf, transition);

    sprintf(buf, "%s{", buf);
    while(0 < list_size(to_l)) {
        sprintf(buf, "%s%zu, ", buf, list_get_first(to_l));
        list_pop_front(to_l);
    }
    sprintf(buf, "%s}", buf);
    dprintf("%s\n", buf);
    return buf;
}