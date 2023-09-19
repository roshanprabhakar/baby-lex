#include <stdlib.h>
#include <stdio.h>

#include "automaton.h"
#include "regex.h"
#include "arrbuf.h"
#include "queue.h"

void init_state(struct state *s)
{
	for (int i = 0; i < sizeof(s->group_connections) / sizeof(struct queue); ++i)
	{
		init_queue(s->group_connections + i, sizeof(struct state *), 5);
	}
	init_queue(&s->nil_connections, sizeof(struct state *), 10);
}

void destroy_state(struct state *s)
{
	for (int i = 0; i < sizeof(s->group_connections) / sizeof(struct queue); ++i)
	{
		destroy_queue(s->group_connections + i);
	}
	destroy_queue(&s->nil_connections);
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
				queue_push(&f->nil_connections, i);
		}
		else if (p->op_right.unary == '?')
		{
				queue_push(&i->nil_connections, f);
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
			queue_push((i->group_connections) + (p->op_left.alphabet - 'A'), f);
	}
	else
	{
		int states_needed = build_regex_automaton(p->op_left.sub_tree, bank, i, f);
		if (states_needed == -1) return -1;
		return states_needed;
	}
	return 0;
}

