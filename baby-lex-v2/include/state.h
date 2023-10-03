#ifndef NFA_STATE_H
#define NFA_STATE_H

#include "queue.h"

/* States are able to connect to any number of other states. The bridge may be a single
 * character, or a character grouping (upper, lower, letter, digit, etc.). Multiple states
 * may be connected to the given state on the same character/grouping. The preliminary
 * groups are defined below:
 *
 * 	lowercase letters: -1 '\xff' [a-z]
 * 	uppercase letters: -2 '\xfe' [A-Z]
 * 	letters: 					 -3 '\xfd' [a-zA-Z]
 * 	digits: 					 -4 '\xfc' [0-9]
 * 	unary operators:	 -5 '\xfb' [...]
 * 	binary operators:	 -6 '\xfa' [...]
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

// Interact with a state's char_queues (add/get).
struct char_queue *state_get_char_queue(struct state *s, char c);
void state_insert_char_queue(struct state *s, char c);

#endif // NFA_STATE_H
