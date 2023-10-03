#include "queue.h"
#include "arrbuf.h"
#include "regex.h"
#include "bitmap.h"

#include "state.h"
#include "state_set.h"

// Given a pointer to an initial state, a pointer to a final state,
// a bank of allocable states, and a regex parse tree, this function constructs the
// automaton responsible for recognizing the passed regex. The value stored at the
// state pointers becomes the start and end state of the automaton. If the pointers are
// null the automaton is not constructed, instead the number of states needed by the
// complete automaton is returned.
int build_regex_automaton(struct regex_parse_tree *p, struct buffer *bank,
		struct state *i, struct state *f);

// Traversing the automaton //
int move_on_nil(struct state *s, struct state_set *set);
int move_set_on_nil(struct queue *states, int num_total_states);
int move_on_alpha(struct state *s, char alpha, struct state_set *set);
int move_set_on_alpha(struct queue *states, char alpha, int num_total_states);

int move(struct queue *i_state, char const *in, int num_total_states);
