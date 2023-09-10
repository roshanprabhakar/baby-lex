#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtable.h"
#include "arrbuf.h"

// Data: [ptr, token (int), attr (int)]*
static struct buffer symtable;

// Data: [char*, 0]*
static struct buffer lexeme_arr;


static char const *int_lexeme = "int";
static char const *long_lexeme = "long";
static char const *plus_lexeme = "+";
static char const *minus_lexeme = "-";
static char const *eof_lexeme = "\xff";

// Preload the symbol table with known lexemes and their tokens.
// Load {, }, (, ), fn
void st_init()
{
	symtable.p = malloc(50); symtable.capacity = 50;
	lexeme_arr.p = malloc(100); lexeme_arr.capacity = 100;

	struct sym_entry sym_entries[] = {
		{int_lexeme, strlen(int_lexeme), SYM_TYPE, 0},
		{long_lexeme, strlen(long_lexeme), SYM_TYPE, 0},
		{plus_lexeme, 1, SYM_OP, 0},
		{minus_lexeme, 1, SYM_OP, 0},
		{eof_lexeme, 1, SYM_EOF, 0}
	};

	for (int i = 0; i < sizeof(sym_entries) / sizeof(struct sym_entry); ++i)
	{
		buffer_insert(&symtable, 
									(struct sym_entry *) sym_entries + i, 
									sizeof(struct sym_entry));
	}
}

struct sym_entry *st_insert(char *lexeme, int token, int val)
{
	long lex_len = strlen(lexeme);
	char *sym_loc = buffer_insert(&lexeme_arr, lexeme, lex_len + 1);
	struct sym_entry entry = {sym_loc, lex_len, token, val};
	return (struct sym_entry *) buffer_insert(&symtable, &entry, sizeof(struct sym_entry));
}

struct sym_entry *st_lookup(char const *lex)
{
	char found = 0;
	for (long i = 0; i < symtable.write_curs / sizeof(struct sym_entry); ++i)
	{
		if (strcmp(((struct sym_entry *) symtable.p)[i].lexeme, lex) == 0)
		{
			return ((struct sym_entry *) symtable.p) + i;
		}
	}
	return NULL;
}

void st_free() { free(symtable.p); free(lexeme_arr.p); }

void st_dump()
{
	printf("lexeme wc: %ld, lexeme cap: %ld\n", 
			lexeme_arr.write_curs, 
			lexeme_arr.capacity);

	printf("symtable wc: %ld, symtable cap: %ld\n",
			symtable.write_curs,
			symtable.capacity);

	for (long i = 0; i < symtable.write_curs / sizeof(struct sym_entry); ++i)
	{
		printf("lexeme: %s, len: %d, token: %d, attr: %d\n",
				((struct sym_entry *) symtable.p)[i].lexeme,
				((struct sym_entry *) symtable.p)[i].lexeme_len,
				((struct sym_entry *) symtable.p)[i].token,
				((struct sym_entry *) symtable.p)[i].attr);
	}
}
