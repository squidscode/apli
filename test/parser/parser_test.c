#include "../testlib/testlib.h"
#include "../../src/parser/parser.h"

/*

<bnf> "Arithmetic Expressions" {
expression = term  { ("+" | "-") term} .
... expression = term
... expression = term ("+" | "-") term
term       = factor  { ("*"|"/") factor} .
... term = factor
... term = factor ("*" | "/") factor
factor     = number | variable | "("  expression  ")" .
number     = r" [123456789][0123456789]* "
} </bnf>

*/

int main() {
    Terminal expression     = non_terminal_from("expression");
    Terminal term           = non_terminal_from("term");
    Terminal factor         = non_terminal_from("factor");
    Terminal number         = terminal_from("NUMBER");
    Terminal plus           = terminal_from("PLUS");
    Terminal minus          = terminal_from("MINUS");
    Terminal multi          = terminal_from("STAR");
    Terminal div            = terminal_from("FORWARD_SLASH");
    Terminal open_paren     = terminal_from("OPEN_PAREN");
    Terminal close_paren    = terminal_from("CLOSE_PAREN");

    /**
     *  --- Arithmetic Rules ---
     * <expr>   := <term>
     * <expr>   := <expr> ('+' | '-') <term>
     * <term>   := <factor>
     * <term>   := <term> ('*' | '/') <factor>
     * <factor> := NUMBER | '(' <expression> ')'
     *
     */
    BnfRules *arithmetic_rules = bnf_rules_new();

    // Expression (multiple terms separated by '+' or '-') rules:
    bnf_rules_add_rule(arithmetic_rules, bnf_rule_from(expression, term));
    bnf_rules_add_rule(arithmetic_rules, bnf_rule_from(expression, expression, plus, term));
    bnf_rules_add_rule(arithmetic_rules, bnf_rule_from(expression, expression, minus, term));

    // Term (multiple factors separated by '*' or '/') rules:
    bnf_rules_add_rule(arithmetic_rules, bnf_rule_from(term, factor));
    bnf_rules_add_rule(arithmetic_rules, bnf_rule_from(term, term, multi, factor));
    bnf_rules_add_rule(arithmetic_rules, bnf_rule_from(term, term, div, factor));

    // Factor (a number or a group -- defined as an '(' <expr> ')') rules:
    bnf_rules_add_rule(arithmetic_rules, bnf_rule_from(factor, number));
    bnf_rules_add_rule(arithmetic_rules, bnf_rule_from(factor, open_paren, expression, close_paren));

    /**
     * - TOKENS: 
     *   - NUMBER: r" [123456789][0123456789]* " [1,1]
     *   - PLUS: r"+"
     *   - MINUS: r"-"
     *   - STAR: r"*"
     *   - FORWARD_SLASH: r"/"
     *   - OPEN_PAREN: r"("
     *   - CLOSE_PAREN: r")"
     */

    TokenRules *tr = token_rules_new();
    token_rules_add_rule_offset(tr, "NUMBER", 0, 0, "(-[123456789][0123456789]*)");
    token_rules_add_rule_offset(tr, "NUMBER", 0, 0, "(\\+?[123456789][0123456789]*|0)");
    token_rules_add_rule(tr, "PLUS", "\\+");
    token_rules_add_rule(tr, "MINUS", "-");
    token_rules_add_rule(tr, "STAR", "\\*");
    token_rules_add_rule(tr, "FORWARD_SLASH", "/");
    token_rules_add_rule(tr, "OPEN_PAREN", "\\(");
    token_rules_add_rule(tr, "CLOSE_PAREN", "\\)");
    token_rules_compile(tr);

    bnf_rules_construct_parse_tree(arithmetic_rules, token_rules_tokenize(tr, "1 + 1"));
    bnf_rules_construct_parse_tree(arithmetic_rules, token_rules_tokenize(tr, "1 - 1"));
    bnf_rules_construct_parse_tree(arithmetic_rules, token_rules_tokenize(tr, "1 * 1"));
    bnf_rules_construct_parse_tree(arithmetic_rules, token_rules_tokenize(tr, "1 / 1"));
    bnf_rules_construct_parse_tree(arithmetic_rules, token_rules_tokenize(tr, "(1 + 1) / 1"));
    bnf_rules_construct_parse_tree(arithmetic_rules, token_rules_tokenize(tr, "(1 + 1) / 1*2"));
    bnf_rules_construct_parse_tree(arithmetic_rules, token_rules_tokenize(tr, "(1 + 1) / 1 * 5 + 3*2"));
    bnf_rules_construct_parse_tree(arithmetic_rules, token_rules_tokenize(tr, "1 + 1 + 1 + 1 + 1"));
}