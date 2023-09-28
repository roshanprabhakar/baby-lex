#include <stdlib.h>
#include <stdio.h>

#include "automaton.h"

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

	// Give ids to all allocable states. NOTE: ID = 1 IS THE ONLY ACCEPTING STATE.
	for (int i = 0; i < num_states; ++i)
	{ ((struct state *)b_automaton.p)[i].id = i; }

	struct state *i = buffer_alloc(&b_automaton, sizeof(struct state)); init_state(i);
	struct state *f = buffer_alloc(&b_automaton, sizeof(struct state)); init_state(f);
	(void) build_regex_automaton(b_parse_tree.p, &b_automaton, i, f);

	/*
	// Dump automaton state.
	for (int i = 0; i < b_automaton.write_curs / sizeof(struct state); ++i)
	{
		struct state *s = ((struct state *)b_automaton.p) + i;
		dump_state(s);
	}
	*/

	char const *in = "BABBABABABBB";

	struct state *i_state = (struct state *)b_automaton.p;

	struct queue init_state;
	init_queue(&init_state, sizeof(struct state *), num_states);
	queue_push(&init_state, &i_state);

	char const *out = move(&init_state, in, num_states);	

	printf("lexeme: ");
	for (char const *c = in; c != out + 1; ++c)
	{
		printf("%c", *c);
	}
	printf("\n");
	printf("num chars in token: %ld\n", out - in + 1);

	/*
	// Sandbox
	struct state *src = (struct state *)b_automaton.p + 3;
	struct state *dst1 = (struct state *)b_automaton.p + 1;
	struct state *dst2 = (struct state *)b_automaton.p + 2;

	queue_push(&src->nil_connections, &dst1);
	queue_push(&src->nil_connections, &dst2);

	src = (struct state *)b_automaton.p + 2;
	dst1 = (struct state *)b_automaton.p + 5;

	queue_push(&src->nil_connections, &dst1);

	// Dump automaton state.
	for (int i = 0; i < b_automaton.write_curs / sizeof(struct state); ++i)
	{
		struct state *s = ((struct state *)b_automaton.p) + i;
		dump_state(&s);
	}

	struct queue states;
	init_queue(&states, sizeof(struct state *), num_states);

	src = (struct state *)b_automaton.p + 0;
	queue_push(&states, &src);

	src = (struct state *)b_automaton.p + 2;
	queue_push(&states, &src);

	move_set_on_alpha(&states, 'A', num_states);
	dump_queue(&states, &dump_state);

	destroy_queue(&states);
	*/




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
