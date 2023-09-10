#include <stdio.h>

#include "symtable.h"
#include "lexan.h"

int main(int argc, char *argv[])
{
	if (argc != 2) return 0;

	st_init();
	FILE *f = fopen(argv[1], "r");

	struct sym_entry *se;
	while (1)
	{
		se = lexan(f);
		printf("%s ", se->lexeme);
		if (se->token == SYM_EOF) break;
	}
	printf("\n");

	st_free();
}
