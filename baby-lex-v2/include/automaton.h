#include "queue.h"
#include "arrbuf.h"
#include "regex.h"
#include "bitmap.h"

/* States are able to connect to any number of other states. The bridge may be a single
 * character, or a character grouping (upper, lower, letter, digit, etc.). Multiple states
 * may be connected to the given state on the same character/grouping. The preliminary 
 * groups are defined below:
 * 	
 * 	lowercase letters: -1 '\xff' [a-z]
 * 	uppercase letters: -2 '\xfe' [A-Z]
 * 	letters: 					 -3 '\xfd' [a-zA-Z]
 * 	digits: 					 -4 '\xfc' [0-9]
 * 	unary: 						 -5 '\xfb' [...]
 * 	binary: 					 -6 '\xfa' [...]
 *
 * Each state contains a queue entry for each of these groups, as well as a separate queue
 * for connections on nil. */

struct char_queue
{
	char c;
	struct queue q; // Queue of states.
};

// One row of the state table defined in the book.
struct state
{
	// (\xff + 1) * -1 -> 0, (\xfe + 1) * -1 -> 1, ...
	struct queue group_connections[6];
	struct queue nil_connections;
	struct queue char_queues; // Queue of char_queue.
	int id;
};

// Initialize all queues in s.
void init_state(struct state *s);

// Free all dynamic memory owned by s.
void destroy_state(struct state *s);

// For debug purposes.
void dump_state(void *state);

// Given a double pointer to an initial state, a double pointer to a final state,
// a bank of allocable states, and a regex parse tree, this function constructs the
// automaton responsible for recognizing the passed regex. The value stored at the
// state pointers becomes the start and end state of the automaton. If the pointers are
// null the automaton is not constructed, instead the number of states needed by the
// complete automaton is returned.
int build_regex_automaton(struct regex_parse_tree *p, struct buffer *bank,
		struct state *i, struct state *f);

// Traversing the automaton //

struct state_set
{
	struct queue states;
	struct bitmap contained_ids;
};

void init_state_set(struct state_set *set, int num_states);
void destroy_state_set(struct state_set *set);
void state_set_add_state(struct state_set *set, struct state *s);
int state_set_contains_state(struct state_set *set, struct state *s);

int move_on_nil(struct state *s, struct state_set *set);
int move_set_on_nil(struct queue *states, int num_total_states);
int move_on_alpha(struct state *s, char alpha, struct state_set *set);
int move_set_on_alpha(struct queue *states, char alpha, int num_total_states);

int move(struct queue *i_state, char const *in, int num_total_states);
