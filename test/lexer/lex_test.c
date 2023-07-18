#include "../testlib/testlib.c"
#include <stdio.h>
#include <string.h>
#include "../../src/lexer/lexer.c"

const char* ftoca(const char* file_path);

#define FGREEN  "\x1b[32;1m"
#define FYELLOW  "\x1b[33;1m"
#define FBLUE  "\x1b[34;1m"
#define FMAGENTA  "\x1b[35;1m"
#define FCYAN  "\x1b[36;1m"
#define FLBLUE  "\x1b[38;2;50;175;255;1m"

// Yes, I'm lazy, I should add this to the regex engine... 
// TODO ^^
#define ALPHANUMERIC "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"

typedef const char* str_ptr;
define_map(str_ptr, str_ptr);


int main() {
    /* Lex a C-like file: */
    const char *contents = ftoca("./src/lexer/regex.c");
    TokenRules *token_rules = tr_new();
    Map(str_ptr, str_ptr) *colors = map_new(str_ptr, str_ptr);
    const char* string_literal = "string-literal";  map_insert(colors, string_literal, "\x1b[38;2;110;207;255;1m");
    const char* f_include = "f-include";            map_insert(colors, f_include, "\x1b[38;2;110;207;255;1m");
    const char* comment = "comment";                map_insert(colors, comment, "\x1b[38;2;100;100;100;1m");
    const char* character = "character";            map_insert(colors, character, FLBLUE);
    const char* open_brace = "open_brace";          map_insert(colors, open_brace, "\x1b[38;2;0;100;100;1m");
    const char* close_brace = "close_brace";        map_insert(colors, close_brace, "\x1b[38;2;0;100;100;1m");
    const char* open_paren = "open_paren";          map_insert(colors, open_paren, FGREEN);
    const char* close_paren = "close_paren";        map_insert(colors, close_paren, FGREEN);
    const char* open_brack = "open_brack";          map_insert(colors, open_brack, FRED);
    const char* close_brack = "close_brack";        map_insert(colors, close_brack, FRED);
    const char* special_operators = "spec_op";      map_insert(colors, special_operators, "\x1b[38;2;198;255;200;1m");
    const char* type = "type";                      map_insert(colors, type, "\x1b[38;2;50;175;0;1m");
    const char* keyword = "keyword";                map_insert(colors, keyword, "\x1b[38;2;0;175;175;1m");
    const char* operator = "operator";              map_insert(colors, operator, "\x1b[38;2;198;244;255;1m");
    const char* number = "number";                  map_insert(colors, number, "\x1b[38;2;255;244;198;1m");
    const char* semicolon = "semicolon";            map_insert(colors, semicolon, "\x1b[38;2;220;220;220;1m");
    const char* fn_identifier = "fn-identifier";    map_insert(colors, fn_identifier, "\x1b[38;2;255;50;0;1m");
    const char* identifier = "identifier";          map_insert(colors, identifier, "\x1b[38;2;255;165;0;1m");
    tr_add_rule(token_rules, comment, 0, 0, "//[^\n]*\n|/\\*[^]*\\*/");
    tr_add_rule(token_rules, string_literal, 1, 0, "[^\\]\"[^\n]*[^\\]\"");
    tr_add_rule(token_rules, f_include, 0, 1, "<["ALPHANUMERIC".]+>[ \r\t]*\n");
    tr_add_rule(token_rules, character, 0, 0, "'\\\\?[^]'");
    tr_add_rule(token_rules, semicolon, 0, 0, ";");
    tr_add_rule(token_rules, open_brace, 0, 0, "\\{");
    tr_add_rule(token_rules, close_brace, 0, 0, "\\}");
    tr_add_rule(token_rules, open_paren, 0, 0, "\\(");
    tr_add_rule(token_rules, close_paren, 0, 0, "\\)");
    tr_add_rule(token_rules, open_brack, 0, 0, "\\[");
    tr_add_rule(token_rules, close_brack, 0, 0, "\\]");
    tr_add_rule(token_rules, type, 1, 1, "[^"ALPHANUMERIC"]const (size_t|void|char|int)[^"ALPHANUMERIC"]");
    tr_add_rule(token_rules, type, 1, 1, "[^"ALPHANUMERIC"](size_t|void|char|int)[^"ALPHANUMERIC"]");
    tr_add_rule(token_rules, keyword, 0, 0, "(while|for|return|#include|#define|#ifndef|#endif|#ifdef|typedef)");
    tr_add_rule(token_rules, special_operators, 0, 0, "(\\*|&)");
    tr_add_rule(token_rules, operator, 0, 0, "(==|!=|<=|>=|->|\\+=|-=|\\+\\+|--|&&|<<|>>)");
    tr_add_rule(token_rules, number, 0, 0, "(-[123456789][0123456789]*)");
    tr_add_rule(token_rules, number, 0, 0, "(\\+?[123456789][0123456789]*|0)");
    tr_add_rule(token_rules, operator, 0, 0, "(=|<|>|.|\\+|-)");
    tr_add_rule(token_rules, fn_identifier, 1, 1, "[^#"ALPHANUMERIC"][#"ALPHANUMERIC"]+[ ]*\\(");
    tr_add_rule(token_rules, identifier, 1, 1, "[^#"ALPHANUMERIC"][#"ALPHANUMERIC"]+[^#"ALPHANUMERIC"]");
    tr_compile(token_rules);
    List(_token_t) *tokens = tr_tokenize(token_rules, contents);

    // Prints the file with tokens highlighted
    size_t size = strlen(contents); size_t num_discarded = 0; size_t num_tokens = list_size(tokens);
    for(size_t i = 0; i < size; ++i) {
        if(list_size(tokens) && contents + i == list_get_front(tokens).ptr)
            printf("%s", map_at(colors, list_get_front(tokens).name));
        printf("%c", contents[i]);
        if(list_size(tokens) && contents + i == list_get_front(tokens).ptr + list_get_front(tokens).length - 1)
            printf("" RESET);
        while(list_size(tokens) && contents + i >= list_get_front(tokens).ptr + list_get_front(tokens).length - 1)
            (num_discarded++, list_pop_front(tokens));
    }
    // Checking if tokens were well-formed
    // printf("\ntokens left: %zu, num_discarded: %zu\n", list_size(tokens), num_tokens - num_discarded);
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