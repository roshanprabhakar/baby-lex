#include <stdlib.h>
#include <stdio.h>

#include "regex.h"

/* Regex CFG:
 *  regex -> term '|' regex | term
 *  term -> factor term | factor
 * 	factor -> atom* | atom? | atom
 *  atom -> (regex) | alphabet 
 *
 * regex - (term, [regex | null])
 * term - (factor |  [term | null])
 * factor - (atom, [* | ? | null])
 * atom - ([(regex) | alphabet], null)
 *
 * Nonterminal functions:
 * 	Given a pointer to an input buffer: consume the input, advancing the pointer
 * 	to the proceeding nonterminal. Do not construct a parse tree, simply dump
 * 	the layout of the nonterminals to stdout.
 *  */

// TODO failure routine currently results in memory leak. Free the given buffer if
// memory allocation fails.

// Each nonterminal function returns the number of nodes including its root exist
// in its parse tree. We can use this to recursively determine how many nodes
// we need to allocate for the full regex's parse tree.
int regex(char **, struct buffer *, struct regex_parse_tree **);


static int term(char **, struct buffer *, struct regex_parse_tree **);
static int factor(char **, struct buffer *, struct regex_parse_tree **);
static int atom(char **, struct buffer *, struct regex_parse_tree **);


/* Given a reference to a char *, advance the char * until it points to a non
 * delimiting character. */
static void strip_head(char **in) { while (**in == ' ' || **in == '\t' || **in == '\n') ++(*in); }


/* is_alphabet(c) is true if c belongs to:
 * 	groups: 	\d-1,\d-2,\d-3,\d-4
 * 	ascii: 		\d32-\d126
 *
 * 	c not equal to: |,(,)
 */
static int is_alphabet(char c)
{
	int ret = 0;
	ret += (c >= -4 && c <= -1) ? 1 : 0;
	if (c >= 32 && c <= 126)
	{
		if (c != 40 && c != 41 && c != 124)
			++ret;
	}
	return ret;
}


/* In a regex_parse_tree of type NODE_ATOM, we determine whether the op_left holds a char
 * based on the value of op_right.holds_alpha. This works since for atom nodes, the right
 * operand is not reserved to hold anything else. A false holds_alpha indicates the left
 * operand holds a pointer to a regex node. */
static int atom(char **in, struct buffer *b, struct regex_parse_tree **root)
{
	// printf("DEBUG: atom, lookahead: %c\n", **in);

	int ret = 1;
	char *in_ref = *in;

	if (root)
	{
		// Request space for an atom node, and construct it.
		*root = buffer_alloc(b, sizeof(struct regex_parse_tree));
		if (*root == NULL) { printf("Failed to allocate node for atom. "); exit(0); }
		(*root)->type = NODE_ATOM;
	}

	// If the lookahead if '(', advance and consume a regex, and assert that the next lookahead
	// is '). Otherwise, consume an alphabet.
	if (*in_ref == '(')
	{
		if (root) { (*root)->op_right.holds_alpha = 0; }

		++in_ref;

		strip_head(&in_ref);
		ret += regex(&in_ref, b, (root) ? &((*root)->op_left).sub_tree : NULL);
		strip_head(&in_ref);

		if (*in_ref != ')')
	{
			printf("ERROR: atom parse: no matching close bracket. ");
			printf("Rest of input: %s\n", in_ref);
			exit(0);
		}
	}
	else if (is_alphabet(*in_ref))
	{
		if (root) // NOTE: cannot be merged into (is_alphabet && root), since the else should only
							// be triggered if !is_alphabet
		{
			(*root)->op_right.holds_alpha = 1;
			(*root)->op_left.alphabet = *in_ref;
		}
	}
	else
	{ 
		printf("ERROR: atom parse: alphabet expected but found: %d %c\n", *in_ref, *in_ref); 
		exit(0); 
	}

	++in_ref;

	*in = in_ref;
	return ret;
}


static int factor(char **in, struct buffer *b, struct regex_parse_tree **root)
{
	// printf("DEBUG: factor, lookahead: %c\n", **in);

	int ret = 1;
	char *in_ref = *in;

	if (root)
	{
		// Request space for a factor node, and construct it.
		*root = buffer_alloc(b, sizeof(struct regex_parse_tree));
		if (*root == NULL) { printf("Failed to alocate node for factor. "); exit(0); }
		(*root)->type = NODE_FACTOR;
	}

	// Consume an atom.
	strip_head(&in_ref);
	ret += atom(&in_ref, b, (root) ? &((*root)->op_left).sub_tree : NULL);
	strip_head(&in_ref);

	// If a unary operator follows, record it in the new node.
	if (*in_ref == '*' || *in_ref == '?')
	{
		if (root) 
		{ (*root)->op_right.unary = *in_ref; }

		++in_ref;
	}
	else if (root)
	{
		(*root)->op_right.unary = 0;
	}

	*in = in_ref;
	return ret;
}


static int term(char **in, struct buffer *b, struct regex_parse_tree **root)
{
	// printf("DEBUG: term, lookahead: %c\n", **in);

	int ret = 1;
	char *in_ref = *in;

	if (root)
	{
		// Request space for a term node, and construct it.
		*root = buffer_alloc(b, sizeof(struct regex_parse_tree));
		if (*root == NULL) { printf("Failed to allocate node for term. "); exit(0); }
		(*root)->type = NODE_TERM;
	}

	// Consume a factor. 
	strip_head(&in_ref);
	ret += factor(&in_ref, b, (root) ? &((*root)->op_left).sub_tree : NULL);
	strip_head(&in_ref);

	// If the next token is a '(' or an alphabet, consume another term.
	if (*in_ref == '(' || is_alphabet(*in_ref)) 
	{ 
		ret += term(&in_ref, b, (root) ? &((*root)->op_right).sub_tree : NULL);
	}
	else if (root)
	{
		(*root)->op_right.sub_tree = NULL;
	}

	// ret state to calling nonterminal.
	*in = in_ref;
	return ret;
}


int regex(char **in, struct buffer *b, struct regex_parse_tree **root)
{
	// printf("DEBUG: regex, lookahead: %c\n", **in);

	// End search if **in is null. 
	if (**in == 0) return 0;

	int ret = 1;
	char *in_ref = *in;

	if (root)
	{
		// Request space for a regex node, and construct it.
		*root = buffer_alloc(b, sizeof(struct regex_parse_tree));
		if (*root == NULL) { printf("Failed to allocate node for regex. "); exit(0); }
		(*root)->type = NODE_REGEX;
	}

	// Consume a term. If it is followed by the union operator, consume another
	// regex.
	strip_head(&in_ref);
	ret += term(&in_ref, b, (root) ? &((*root)->op_left).sub_tree : NULL);
	strip_head(&in_ref);
	
	// If the union operator follows, consume a regex.
	if (*in_ref == '|') 
	{
		++in_ref;
		ret += regex(&in_ref, b, (root) ? &((*root)->op_right).sub_tree : NULL);
	}
	else if (root)
	{
		(*root)->op_right.sub_tree = NULL;
	}

	// return status of internal pointer to calling nonterminal function.
	*in = in_ref;
	return ret;
}


void dump_regex_parse_tree(struct regex_parse_tree *p)
{
	if (p->type == NODE_REGEX)
	{
		printf("(");
		dump_regex_parse_tree(p->op_left.sub_tree);
		if (p->op_right.sub_tree)
		{
			printf("|");
			dump_regex_parse_tree(p->op_right.sub_tree);
		}
		printf(")");
	}
	else if (p->type == NODE_TERM)
	{
		dump_regex_parse_tree(p->op_left.sub_tree);
		if (p->op_right.sub_tree)
		{ dump_regex_parse_tree(p->op_right.sub_tree); }
	}
	else if (p->type == NODE_FACTOR)
	{
		dump_regex_parse_tree(p->op_left.sub_tree);
		if (p->op_right.unary) 
		{ printf("%c", p->op_right.unary); }
	}
	else if (p->type == NODE_ATOM)
	{
		if (p->op_right.holds_alpha)
		{ printf("%c", p->op_left.alphabet); }
		else
		{ dump_regex_parse_tree(p->op_left.sub_tree); }
	}
	else
	{
		printf("ERROR");
	}
}
