#include <stdlib.h>
#include <stdio.h>

#include "lexer.h"
#include "arrbuf.h"

int main()
{
	char *reg_str = "(A(B*)?)ABBA|AAAA";

	int num_nodes = regex(&reg_str, NULL, NULL);
	struct buffer b = 
	{
		.p = malloc(sizeof(struct regex_parse_tree) * num_nodes),
		.write_curs = 0,
		.capacity = sizeof(struct regex_parse_tree) * num_nodes
	};

	struct regex_parse_tree *root;
	(void) regex(&reg_str, &b, &root);

	printf("%d\n", num_nodes);

	// Free entire parse tree in one go.
	free(b.p);

	return 0;
}
