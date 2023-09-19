#include "queue.h"
#include "arrbuf.h"
#include "regex.h"

/* States are able to connect to any number of other states. The bridge may be a single
 * character, or a character grouping (upper, lower, letter, digit, etc.). Multiple states
 * may be connected to the given state on the same character/grouping. The preliminary 
 * groups are defined below:
 * 	
 * 	lowercase letters: -1 '\xff' [a-z]
 * 	uppercase letters: -2 '\xfe' [A-Z]
 * 	letters: 					 -3 '\xfd' [a-zA-Z]
 * 	digits: 					 -4 '\xfc' [0-9]
 *
 * Each state contains a queue entry for each of these groups, as well as a separate queue
 * for connections on nil. */

// One row of the state table defined in the book.
struct state
{
	// TODO: (\xff + 1) * -1 -> 0, (\xfe + 1) * -1 -> 1, ...
	// current: 0 -> A, ... 3 -> D
	struct queue group_connections[4];
	struct queue nil_connections;
	int id;
};

// Initialize all queues in s.
void init_state(struct state *s);

// Free all dynamic memory owned by s.
void destroy_state(struct state *s);

// For debug purposes.
void dump_state(struct state *s);

// Given a double pointer to an initial state, a double pointer to a final state,
// a bank of allocable states, and a regex parse tree, this function constructs the
// automaton responsible for recognizing the passed regex. The value stored at the
// state pointers becomes the start and end state of the automaton. If the pointers are
// null the automaton is not constructed, instead the number of states needed by the
// complete automaton is returned.
int build_regex_automaton(struct regex_parse_tree *p, struct buffer *bank,
		struct state *i, struct state *f);
