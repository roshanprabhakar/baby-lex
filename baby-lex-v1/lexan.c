#include <stdio.h>
#include <ctype.h>

#include "lexan.h"

#define MAX_TOKEN_LEN 100

static char tok_buf[MAX_TOKEN_LEN];

// TODO last token gets returned twice, WHY??? shouldn't be this hard..
// TODO make this able to handle difference file streams.
struct sym_entry *lexan(FILE *f)
{
	static int c; // Lookahead.
	static char has_lookahead = 0;
	if (!has_lookahead)
	{ c = fgetc(f); has_lookahead = 1; }

	int pos = 0, eof = 0;
	do
	{
		if (c == '+' || c == '-' || c == ';' ||
				c == '(' || c == ')' || c == '{' ||
				c == '}' || c == '.' || c == EOF) 
		{
			tok_buf[pos] = c; ++pos; tok_buf[pos] = 0; 
			if (c == EOF) eof = 1;
			c = fgetc(f);

			break;
		}

		if (isalnum(c))
		{
			do
			{
				if (pos < MAX_TOKEN_LEN - 1)
				{ tok_buf[pos] = c; ++pos; }

				c = fgetc(f);
			}
			while (isalnum(c));
			tok_buf[pos] = 0;

			break;
		}

		c = fgetc(f);
	}
	while (c != EOF);

	struct sym_entry *p = st_lookup(tok_buf);
	if (p == NULL)
	{
		if (eof == 1) 			return st_insert(tok_buf, SYM_EOF, 0);
		else if (pos == 1)	return st_insert(tok_buf, SYM_CHAR, 0);
		else 								return st_insert(tok_buf, SYM_ID, 0);
	}
	else return p;
}
