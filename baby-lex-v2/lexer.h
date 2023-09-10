#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

#include "arrbuf.h"

enum regex_parse_node_type { NODE_REGEX, NODE_TERM, NODE_FACTOR, NODE_ATOM };

struct regex_parse_tree
{
	enum regex_parse_node_type type :2;
	union
	{
		struct regex_parse_tree *sub_tree;
		char alphabet;
	} op_left;
	union
	{
		struct regex_parse_tree *sub_tree;
		char unary;
		char holds_alpha;
	} op_right;
};

// Returns the number of nodes in the generated parse tree. If b is not null,
// we assume b has been allocated with enough room for the regex's node + 
// subtree, and we populate the buffer with the parse tree. We also expect that
// the caller wants the root of the created node to be defined in its own scope
// (to support the inherently recursive nature of parse trees), so we populate
// *root with a pointer to the created node.
int regex(char **in, struct buffer *b, struct regex_parse_tree **root);

#endif // LEXER_H
