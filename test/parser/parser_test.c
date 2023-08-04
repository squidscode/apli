#include "../testlib/testlib.h"
#include "../../src/parser/parser.h"

/*

<ebnf> "Arithmetic Expressions" {
expression = term  { ("+" | "-") term} .
... expression = term
... expression = term ("+" | "-") term
term       = factor  { ("*"|"/") factor} .
... term = factor
... term = factor ("*" | "/") factor
factor     = constant | variable | "("  expression  ")" .
variable   = r"(x|y|z)" .
number     = r" [123456789][0123456789]* "
} </ebnf>

*/

int main() {
    Terminal expression     = non_terminal_from("expression");
    Terminal term           = non_terminal_from("term");
    Terminal factor         = non_terminal_from("factor");
    Terminal variable       = non_terminal_from("variable");
    Terminal number         = terminal_from("number");
    Terminal plus           = terminal_from("+");
    Terminal minus          = terminal_from("-");
    Terminal multi          = terminal_from("*");
    Terminal div            = terminal_from("/");
    Terminal open_paren     = terminal_from("(");
    Terminal close_paren    = terminal_from(")");
    Terminal x              = terminal_from("x");
    Terminal y              = terminal_from("y");
    Terminal z              = terminal_from("z");

    EbnfRules *arithmetic_rules = ebnf_rules_new();
    ebnf_rules_add_rule(arithmetic_rules, ebnf_rule_from(expression, term));
    assertTrue(0 == _ebnf_rules_find_minimum_lookahead(arithmetic_rules));
    ebnf_rules_add_rule(arithmetic_rules, ebnf_rule_from(expression, term, plus, term));
    ebnf_rules_add_rule(arithmetic_rules, ebnf_rule_from(expression, term, minus, term));
    assertTrue(1 == _ebnf_rules_find_minimum_lookahead(arithmetic_rules));
}