#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "arrbuf.h"

/* Step 1: 	Given an alphabet and a set of chars, extract the first alphabet
 * 					that occurs as a prefix of a given string. */

// Data: [ptr(name), ptr(data)]*
struct buffer alphabets;

void lexer_init()
{
	alphabets.capacity = sizeof(char *) * 2;
	alphabets.p = malloc(alphabets.capacity);
	longest_alphabet_name = 0;
}

/* Accepts the alphabet named name, with the charset chars. chars is an
 * unordered string representing the set of characters belonging to alphabet
 * name. It must be null terminated. Copies are NOT created, the data pointed
 * to must live for the duration of the lexer. */
void accept_alpha(char const *name, char const *chars)
{
	buffer_insert(&alphabets, &name, sizeof(char *));
	buffer_insert(&alphabets, &chars, sizeof(char *));
	buffer_insert(&alphabets, -1, sizeof(char)); // Flags on this entry.
}

/* Returns a pointer to the charset of the alphabet whose name appears as 
 * a prefix in the argument. If none are found, returns null. *in is advanced
 * to the character directly following the first alphabet name. */
char const *get_charset(char const **in)
{
	struct entry
	{
		char const *name;
		char const *charset;
		char flags;
	};

	int i;
	for (; (*in)[i]; ++i, ++(*in))
	{
		for (int i = 0; i < alphabets.write_curs / sizeof(struct entry); ++i)
		{
			struct entry *e = ((struct entry *) alphabets.p) + i;
			if (e->flags & 1 == 0) continue;

			if (e->name[i] == 0)
			{ return e->charset; }

			else if (e->name[i] != (*in)[i])
			{	e->flags &= (-1 ^ 1); }

		}
	}
	if ((*in)[i] == 0) return NULL;
}

