#include <stdlib.h>
#include <stdio.h>

#include "automaton.h"
#include "regex.h"
#include "arrbuf.h"
#include "queue.h"
#include "bitmap.h"

  //////////////////////////////////////////////////////////////////////
 ///////////////// CONSTRUCTING AND INSPECTING STATES /////////////////
//////////////////////////////////////////////////////////////////////

void init_state(struct state *s)
{
	for (int i = 0; i < sizeof(s->group_connections) / sizeof(struct queue); ++i)
	{
		init_queue(s->group_connections + i, sizeof(struct state *), 2);
	}
	init_queue(&s->nil_connections, sizeof(struct state *), 5);
}

void destroy_state(struct state *s)
{
	for (int i = 0; i < sizeof(s->group_connections) / sizeof(struct queue); ++i)
	{
		destroy_queue(s->group_connections + i);
	}
	destroy_queue(&s->nil_connections);
}

static void dump_state_queue_entry(void *p)
{
	printf("%d ", (*((struct state **)p))->id);
}

/* This function takes a double pointer to a state, and is meant for 
 * inspecting the content of queues (typically pointers). */
void dump_state(void *p)
{
	struct state *s = *(struct state **)p;
	printf("------- STATE %d -------\n", s->id);
	for (int i = 0; i < sizeof(s->group_connections) / sizeof(struct queue); ++i)
	{
		struct queue *q = s->group_connections + i;
		printf("%ld group %c connections: ", queue_length(q), 'A' + i);
		dump_queue(q, &dump_state_queue_entry);
		printf("\n");
	}
	struct queue *q = &s->nil_connections;
	printf("%ld nil connections: ", queue_length(q));
	dump_queue(q, &dump_state_queue_entry);
	printf("\n");
}

static int build_term_automaton(struct regex_parse_tree *, struct buffer *,
		struct state *, struct state *);
static int build_factor_automaton(struct regex_parse_tree *, struct buffer *,
		struct state *, struct state *);
static int build_atom_automaton(struct regex_parse_tree *, struct buffer *,
		struct state *, struct state *);

/* Regex automaton:
 * 	Construct the term sub automaton on the given states. If it is followed
 * 	by a regex, construct two new states and build another regex automaton on
 * 	those new states. Connect the initial i to the new i, as well as the 
 * 	initial f to the new f on nil. All proceeding automaton functions return
 * 	the number of states their subtrees create, and no automaton is constructed
 * 	if null pointers are passed.
 */
int build_regex_automaton(struct regex_parse_tree *p, struct buffer *bank, 
		struct state *i, struct state *f)
{
	if (p->type != NODE_REGEX)
	{
		printf("Building regex automaton on non-regex parse tree, aborting.\n");
		return -1;
	}
	
	char construct = bank && i && f;
	int ret = 0;
	int states_needed;

	states_needed = build_term_automaton(p->op_left.sub_tree, bank, i, f);
	if (states_needed == -1) return -1;
	ret += states_needed;

	if (p->op_right.sub_tree)
	{
		struct state *new_i, *new_f;
		if (construct)
		{
			new_i = buffer_alloc(bank, sizeof(struct state)); init_state(new_i);
			new_f = buffer_alloc(bank, sizeof(struct state)); init_state(new_f);

			// Connect i -> new_i and new_f -> f on nil.
			queue_push(&i->nil_connections, &new_i);
			queue_push(&new_f->nil_connections, &f);
		}

		states_needed = build_regex_automaton(
				p->op_right.sub_tree, 
				bank, 
				(construct) ? new_i : i, 
				(construct) ? new_f : f
		);
	
		if (states_needed == -1) return -1;
		ret += 2 + states_needed;
	}
	return ret;
}


static int build_term_automaton(struct regex_parse_tree *p, struct buffer *bank,
		struct state *i, struct state *f)
{
	if (p->type != NODE_TERM)
	{
		printf("Building term automaton on non-term parse tree, aborting.\n");
		return -1;
	}

	int ret = 0;
	char construct = bank && i && f;
	char new_state_needed = p->op_right.sub_tree != 0;
	int states_needed;

	struct state *target = (new_state_needed && construct) ? 
		buffer_alloc(bank, sizeof(struct state)) : f;

	if (new_state_needed && construct)
		init_state(target);

	states_needed = build_factor_automaton(p->op_left.sub_tree, bank, i, target);
	if (states_needed == -1) return -1;
	ret += states_needed;

	if (new_state_needed)
	{
		states_needed = build_term_automaton(
				p->op_right.sub_tree,
				bank,
				(construct) ? target : i,
				f
		);
		if (states_needed == -1) return -1;
		ret += 1 + states_needed;
	}
	
	return ret;
}


static int build_factor_automaton(struct regex_parse_tree *p, struct buffer *bank,
		struct state *i, struct state *f)
{
	if (p->type != NODE_FACTOR)
	{
		printf("Building factor automaton on non-factor parse tree, aborting.\n");
		return -1;
	}

	char construct = bank && i && f;
	int ret = 0;
	int states_needed;

	states_needed = build_atom_automaton(p->op_left.sub_tree, bank, i, f);
	if (states_needed == -1) return -1;
	ret += states_needed;

	if (construct)
	{
		if (p->op_right.unary == '*')
		{
			queue_push(&f->nil_connections, &i);
		}
		else if (p->op_right.unary == '?')
		{
			queue_push(&i->nil_connections, &f);
		}
		else if (p->op_right.unary != 0)
		{
			printf("Unrecognized factor unary, aborting.\n");
			return -1;
		}
	}

	return ret;
}


static int build_atom_automaton(struct regex_parse_tree *p, struct buffer *bank,
		struct state *i, struct state *f)
{
	if (p->type != NODE_ATOM)
	{
		printf("Building atom automaton on non-atom parse_tree, aborting.\n");
		return -1;
	}

	// Build atom.
	if (p->op_right.holds_alpha)
	{
		// We cannot chain combine the two if statements, as we want to trigger the else
		// regardless of bank && i && f.
		if (bank && i && f)
		{
			queue_push(i->group_connections + (p->op_left.alphabet - 'A'), &f);
		}
	}
	else
	{
		int states_needed = build_regex_automaton(p->op_left.sub_tree, bank, i, f);
		if (states_needed == -1) return -1;
		return states_needed;
	}
	return 0;
}

  /////////////////////////////////////////////////////////////////////
 ///////////////////// TREVERSING THE AUTOMATON //////////////////////
/////////////////////////////////////////////////////////////////////


void init_state_set(struct state_set *set, int num_states)
{
	init_queue(&set->states, sizeof(struct state *), num_states);
	init_bitmap(&set->contained_ids, num_states);
}

void destroy_state_set(struct state_set *set)
{
	destroy_queue(&set->states);
	destroy_bitmap(&set->contained_ids);
}

void state_set_add_state(struct state_set *set, struct state *s)
{
	if (!bitmap_query(&set->contained_ids, s->id))
	{
		bitmap_set(&set->contained_ids, s->id);
		queue_push(&set->states, &s);
	}
}

int state_set_contains_state(struct state_set *set, struct state *s)
{
	return bitmap_query(&set->contained_ids, s->id);
}

/*
 * src: The state we start from.
 * set: The set of states already found on a nil connection. A set is needed
 * 			to ensure move_on_nil does not enter an infinite recursive loop.
 */
void move_on_nil(struct state *src, struct state_set *set)
{
	// If src is not in map, add src to dst, set src in map, and call move_on_nil
	// on all states nil connected to src.
	if (!state_set_contains_state(set, src))
	{
		state_set_add_state(set, src);

		struct queue src_nil_dup = src->nil_connections;
		while (queue_length(&src_nil_dup))
		{
			struct state *s;
			queue_pop(&src_nil_dup, &s);
			move_on_nil(s, set);
		}
	}
}


/* We start with a queue of states. We are going to modify this queue to reflect
 * the move operation. If at the end of this procedure q is empty, then there are
 * no moves possible and the automaton has frozen. q starts containing 1 state:
 * the initial state.
 * 
 * During this process we keep track of the last character that moved the accepting
 * state in q. A pointer to this character is moved into out, and the characters
 * between in and out comprise the matching lexeme.
 */

/* After a move operation, states contains all the states connected to the previous
 * states on *in, as well as all the states connected to the previous states on nil.
 * This means that before the automaton is called on the first char in the input,
 * elementary move must be called on nill.
 */


/*
void move(struct queue *states, char const *in)
{
	struct queue states_dup = *states;
	for (int num_states = queue_length(states); num_states; --num_states)
	{
		struct state *s;
		queue_pop(&states_dup, &s);
		
	}
}


void move(struct queue *states, char const *in)
{
	// Given a pointer to the next character to consume, advance the set of states
	// to the suceeding set.
	
	struct queue tmp; 
	init_queue(&tmp, sizeof(struct state *), 5);

	struct queue *states_dup = states;
	for (int num_states = queue_length(states); num_states; --num_states)
	{
		struct state *s;
		queue_pop(states_dup, &s);
		
		struct queue *char_connections = s->group_connections + (*in - 'A');
		struct queue *nil_connections = &s->nil_connections;


	}

	destroy_queue(&tmp);
}


void move(struct queue *q, char const *in, char const **out)
{
	long num_states = queue_length(q);

	for (char *curs = in; *in && num_states; ++curs)
	{
		for (int i = 0; i < num_states; ++i)
		{
			struct state *s = queue_pop(q);

			// char_connect and nil_connect may be safely popped from. Doing so
			// will not change queue's owned by states.
			struct queue char_connect;
			queue_dup(&char_connect, s->group_connections + (*in - 'A'));

			struct queue nil_connect;
			queue_dup(&nil_connect, &s->nil_connections);
			
			// Push all nil connections into q.
			struct state *bin;
			while (queue_pop(nil_connect, &bin) != -1)
			{ 
				queue_push(q, &bin); 
				if ( // TODO
			}

		}

		num_states = queue_length(q);
	}
}
*/
