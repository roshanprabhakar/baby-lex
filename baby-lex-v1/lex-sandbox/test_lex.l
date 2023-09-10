%{
	/* Copied verbatim. */
	#define IF 0
	#define THEN 1
	#define ELSE 2
	#define ID 3
	#define NUMBER 4
	#define RELOP 5
	#define LT 6
	#define LE 7
	#define EQ 8
	#define NE 9
	#define GT 10
	#define GT 11
	#define GE 12
%}

/* Regular definitions. */
delim 	[\t | \n]
ws			{delim}+
letter	[A-Za-z]
digit 	[0-9]
id			{letter}({letter} | {digit})*

%%

{ws}			{ /* No action, no return. */ }
"if"			{	return IF; }
"then"		{ return THEN; }
"else"		{ return ELSE; }
"="				{ yylval = EQ; return RELOP; }
"<>"			{ yylval = NE; return RELOP; }
">"				{ yylval = GT; return RELOP; }
">="			{ yylval = GE; return RELOP; }

%%

int install_id()
{
	/* Procedure to install lexeme, whose first character is 
		 pointed to by yytext and whose length is yyleng, into
		 the symbol table and return a pointer thereto. */
	return 0; 
}

int install_num()
{
	/* A similar procedure to install a lexeme that is a number. */
	return 1;
}
