#ifndef REGEX_H
#define REGEX_H

#include <string.h>
#include <assert.h>
#ifdef UNOPTIMIZED
#include "nfa.h"
#else
#include "nfa_optimized.h"
#endif

#include "../util/bitset.h"

/**
 * Loads, compiles, and runs a POSIX (ERE) style regex.
 * 
 * ----- Usage -----
 *   Regex *reg = regex_from("...");
 *     - regex_compile(reg)                   -> Regex*
 *     - regex_run(reg, str: const char*)     -> size_t ( 0 or 1 )
 *     - regex_find_all(reg, const char*)     -> List(_regex_match_t)*
 *     - regex_free(reg)                      -> void
 */

#define Regex                       _regex_t
#define regex_from(str)             (_regex_fn_impl_.from((str)))
#define regex_compile(regex)        (_regex_fn_impl_.compile((regex)))
#define regex_run(regex, str)       (_regex_fn_impl_.run((regex), (str)))
#define regex_find_all(regex,str)   (_regex_fn_impl_.find_all_matches((regex), (str)))
#define regex_free(regex)           (_regex_fn_impl_.destroy((regex)))

/**
 * Regex parsing algorithm:
 *   - (1) Split the regex into different capturing groups
 *   ---> a capturing group is a (const char*, size_t) representing
 *        a string segment. Any alternating symbols in the base (ie.)
 *        not nested in any '(' or ')' characters are considered splits
 *        between different capturing groups.
 *   - (2) If len(capturing_groups) == 1:
 *      - EXPAND!
 *   - (3) else:
 *      - for(capture_group : capturing_groups):
 *            add_epsilon(start, next_free_state)
 *            end_states.append(PARSE(capture_group, next_free_state))
 *            next_free_state = end_states[-1] + 1
 *        for each end_state in end_states:
 *            add_epsilon(end_state, next_free_state)
 *        return next_free_state
 */

#define init_regex() \
    define_list(char); \
    define_list(int); \
    init_dfa_types(size_t, char); \
    typedef struct _size_t_char_dfa_ _size_t_char_dfa_t; \
    _size_t_char_dfa_t* _size_t_char_dfa_new(size_t); \
    init_nfa(size_t, char); \
    define_dfa(size_t, char); \
    define_nfa(size_t, char)

struct _regex_;
struct _regex_match_;

struct _regex_match_ {
    size_t begin;
    size_t length;
};
typedef struct _regex_match_ _regex_match_t;


typedef struct _regex_ _regex_t;

typedef struct __regex_match_t_list_ _regex_match_t_list_t;
struct _regex_fns_ {
    struct _regex_* (*from)(const char*);
    struct _regex_* (*compile)(struct _regex_*);
    size_t (*run)(struct _regex_*, const char*);
    List(_regex_match_t)* (*find_all_matches)(struct _regex_*, const char*);
    void (*destroy)(struct _regex_*);
};
typedef struct _regex_fns_ _regex_fns_t;

/* The virtual table for the regex functions are exposed, if the user wants to 
   override one of the regex functions. */
extern _regex_fns_t _regex_fn_impl_;

#include "regex.c"

#endif