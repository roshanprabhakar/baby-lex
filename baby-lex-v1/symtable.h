#ifndef SYMTABLE_H
#define SYMTABLE_H

#define SYM_TYPE 256
#define SYM_OP 257
#define SYM_ID 258
#define SYM_CHAR 258
#define SYM_EOF -1

struct sym_entry
{
	char const *lexeme;
	int lexeme_len;
	int token;
	int attr;
};

// Initializes the symbol table with with all known token lexemes.
void st_init(void);

// Inserts the given lexeme into the symbol table, of length len
// and with attribute attr. This function should be used of the passed
// pointer is a temporary. For RO strings, manual insertion should be
// performed. A pointer to the inserted sym_entry is returned.
struct sym_entry *st_insert(char *lexeme, int token, int attr);

// Returns the symbol table entry corresponding to the given lexeme.
struct sym_entry *st_lookup(char const *lexeme);

// Frees memory occupied by the symbol table.
void st_free(void);

// Dumps the contents of the symbol table.
void st_dump(void);

#endif // SYMTABLE_H
