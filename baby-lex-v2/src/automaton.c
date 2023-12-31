#include <stdlib.h>
#include <stdio.h>

#include "automaton.h"
#include "regex.h"
#include "arrbuf.h"
#include "queue.h"
#include "bitmap.h"
#include "char_groups.h"
#include "state.h"
#include "state_set.h"

  //////////////////////////////////////////////////////////////////////
 ///////////////// CONSTRUCTING AND INSPECTING STATES /////////////////
//////////////////////////////////////////////////////////////////////

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
			char alpha = p->op_left.alphabet;
			if (alpha >= 32 && alpha <= 126)
			{
				// Create/Append to the char queue for this character.
				struct char_queue *cq = state_get_char_queue(i, alpha);
				if (cq == 0x0)
				{
					state_insert_char_queue(i, alpha);
					cq = state_get_char_queue(i, alpha);
				}
				queue_push(&cq->q, &f);
			}
			else if (alpha <= 0 && alpha >= -7)
			{
				// We are dealing with a character groupings
				queue_push(i->group_connections - (alpha + 1), &f);
			}
			else
			{
				printf("Encountered unknown character or grouping: %c, aborting.\n", alpha);
				return -1;
			}

			// queue_push(i->group_connections + (p->op_left.alphabet - 'A'), &f);
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

/*
 * src: The state we start from.
 * set: The set of states already found on a nil connection. A set is needed
 * 			to ensure move_on_nil does not enter an infinite recursive loop.
 *
 * If the final state is reached on this move, return 1. Else return 0.
 */
int move_on_nil(struct state *src, struct state_set *set)
{
	int ret = 0;

	if (src->id == 1) ret = 1;

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
			if (move_on_nil(s, set)) ret = 1;
		}
	}

	return ret;
}

/* states: A queue of states representing the initial condition, and modified
 * by the procedure to reflect the move on e-connections. */
int move_set_on_nil(struct queue *states, int num_total_states)
{
	int ret = 0;

	// We are going to add to this iteratively in a BFS search from states,
	// then extract the underlying queue.
	struct state_set reachable;
	init_state_set(&reachable, num_total_states);
	
	int num_states = queue_length(states);
	for (; num_states; --num_states)
	{
		struct state *cur;
		queue_pop(states, &cur);

		if (move_on_nil(cur, &reachable)) ret = 1;
	}

	destroy_queue(states);
	*states = reachable.states;
	destroy_bitmap(&reachable.contained_ids);

	return ret;
}

int move_on_alpha(struct state *src, char alpha, struct state_set *set)
{
	int ret = 0;

	// Get queue containing all queues from which to add.
	struct queue char_groups;

	// Only needs space for max 1 special queue + 6 group queues.
	init_queue(&char_groups, sizeof(struct queue *), 7);

	struct char_queue *cq = state_get_char_queue(src, alpha);
	if (cq)
	{
		struct queue *unique_char_q = &(cq->q);
		queue_push(&char_groups, &unique_char_q);
	}

	int groups = is_lower(alpha) 			| 
							 is_upper(alpha) 			| 
							 is_letter(alpha) 		|
							 is_hex_letter(alpha) |
							 is_digit(alpha) 			|
							 is_unary(alpha) 			|
							 is_binary(alpha);

	// Check each of the five groups.
	struct queue *buf;
	for (int i = 0; i < 6; ++i)
	{
		int add_group = groups & 1;
		if (add_group)
		{
			buf = src->group_connections + i;
			queue_push(&char_groups, &buf);
		}
		groups >>= 1;
	}

	int num_groups = queue_length(&char_groups);
	for (; num_groups; --num_groups)
	{
		struct queue *group;
		queue_pop(&char_groups, &group);

		struct queue poppable_group = *group;
		int num_states = queue_length(group);
		for (; num_states; --num_states)
		{
			struct state *s;
			queue_pop(&poppable_group, &s);
			state_set_add_state(set, s);
			if (s->id == 1) ret = 1;
		}
	}

	return ret;
}

int move_set_on_alpha(struct queue *states, char alpha, int num_total_states)
{
	int ret = 0;

	struct state_set reachable;
	init_state_set(&reachable, num_total_states);

	int num_states = queue_length(states);
	for (; num_states; --num_states)
	{
		struct state *cur;
		queue_pop(states, &cur);

		if (move_on_alpha(cur, alpha, &reachable)) ret = 1;
	}

	destroy_queue(states);
	*states = reachable.states;
	destroy_bitmap(&reachable.contained_ids);

	return ret;
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
int move(struct queue *states, char const *in, int num_total_states)
{
	int found = move_set_on_nil(states, num_total_states);

	int ret = 0, counter = 0;
	for (char const *c = in; *c && queue_length(states); ++c)
	{
		move_set_on_alpha(states, *c, num_total_states);
		found = move_set_on_nil(states, num_total_states);
		++counter;
		if (found) ret = counter;
	}

	return ret;
}
