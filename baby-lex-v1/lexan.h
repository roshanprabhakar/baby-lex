#ifndef LEXAN_H
#define LEXAN_H

#define MAX_TOKEN_LEN 100

#include "symtable.h"

// Returns the symbol table entry of the next lexeme in the 
// input stream. If f is at EOF, NULL is returned.
struct sym_entry *lexan(FILE *f);

#endif // LEXAN_H
