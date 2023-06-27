#include "../util/map.h"
#include "../util/vector.h"
#include "../util/list.h"
#include "./dfa.c"

#ifndef call
#define call(context, arg)      ((context)->call((context), (arg)))
#endif

/**
 * Token rules are defined as a mapping from a "token name" (const char*)
 * to a *DFA* that accepts inputs for that specific language. 
 * 
 * DFA's that accept a specific language can be encoded as functions that 
 * take in a string (const char*) and return `1` if it accepts or `0` if 
 * it rejects. In order to create a list of tokens from a given string, a 
 * binary-search-like algorithm with a DFA function that accepts if the string
 * **contains** the given token can be used. 
 * 
 * In order to avoid confusion, I will be separating the `function' representation
 * of a DFA from an interal (map-based) representation of a DFA
 *   - An `fdfa_t' is a dfa represented as a function closure (or a C-like representation of a 
 *     function closure)
 *     - TYPE : [context] st.
 *       - call(context, const char*)   ->   size_t (indicating true[1] / false[0])
 *   - A `dfa_t' is a dfa represented through a directed graph. The internal representation
 *     of a `dfa_t' isn't important, because a function that converts a `dfa_t' to a
 *     `fdfa_t' must be supplied.
 * 
 */

struct _token_rule_ {
    const char* name;
    fdfa_t *fdfa;
};

// define_vector();

/**
 * A `_token_rules_' struct contains a vector of 
 */
struct _token_rules_ {

};

typedef struct _token_rules_ token_rules_t;