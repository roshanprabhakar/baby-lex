#include <stdlib.h>
#include <stdio.h>

#include "automaton.h"
#include "regex.h"
#include "arrbuf.h"
#include "queue.h"

int main()
{
	char *reg_str = "(A|B)*AB";
	char *ref2 = reg_str;

	// ----------------------------
	// Allocate and build parse tree.
	// ----------------------------

	int num_nodes = regex(&reg_str, NULL, NULL);
	struct buffer b_parse_tree = 
	{
		.p = malloc(sizeof(struct regex_parse_tree) * num_nodes),
		.write_curs = 0,
		.capacity = sizeof(struct regex_parse_tree) * num_nodes
	};

	struct regex_parse_tree *root;
	(void) regex(&ref2, &b_parse_tree, &root);

	printf("num nodes: %d\n", num_nodes);
	dump_regex_parse_tree(b_parse_tree.p); printf("\n");

	// ----------------------------
	// Allocate and build automaton.
	// ----------------------------

	int num_states = 2 + build_regex_automaton(b_parse_tree.p, NULL, NULL, NULL);
	printf("num states: %d\n", num_states);

	struct buffer b_automaton = 
	{
		.p = malloc(sizeof(struct state) * (num_states)),
		.write_curs = 0,
		.capacity = sizeof(struct state) * (num_states)
	};

	// Give ids to all allocable states.
	for (int i = 0; i < num_states; ++i)
	{ ((struct state *)b_automaton.p)[i].id = i; }

	struct state *i = buffer_alloc(&b_automaton, sizeof(struct state)); init_state(i);
	struct state *f = buffer_alloc(&b_automaton, sizeof(struct state)); init_state(f);
	(void) build_regex_automaton(b_parse_tree.p, &b_automaton, i, f);

	// Dump automaton state.
	for (int i = 0; i < b_automaton.write_curs / sizeof(struct state); ++i)
	{
		struct state *s = ((struct state *)b_automaton.p) + i;
		dump_state(s);
	}

	// ----------------------------
	// Destroy parse tree.
	// Destroy and free all states.
	// ----------------------------

	free(b_parse_tree.p);

	for (int i = 0; i < b_automaton.write_curs / sizeof(struct state); ++i)
	{
		destroy_state(((struct state *)b_automaton.p) + i);
	}
	free(b_automaton.p);

	return 0;
}
