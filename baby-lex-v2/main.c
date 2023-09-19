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

	struct state *i = buffer_alloc(&b_automaton, sizeof(struct state));
	struct state *f = buffer_alloc(&b_automaton, sizeof(struct state));
	(void) build_regex_automaton(b_parse_tree.p, &b_automaton, i, f);

	// Free entire parse tree and automaton in one go.
	free(b_parse_tree.p);
	free(b_automaton.p);

	return 0;
}
