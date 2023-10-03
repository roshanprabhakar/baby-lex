#include "char_groups.h"

int is_lower(char c)
{
	if (c >= 97 && c <=122) return (1 << 0);
	return 0;
}

int is_upper(char c)
{
	if (c >= 65 && c <= 90) return (1 << 1);
	return 0;
}

int is_hex_letter(char c)
{
	if ((c >= 65 && c <= 70) || (c >= 97 && c <= 102)) return (1 << 2);
	return 0;
}

int is_letter(char c)
{
	if (is_lower(c) || is_upper(c)) return (1 << 3);
	return 0;
}

int is_digit(char c)
{
	if (c >= 48 && c <= 57) return (1 << 4);
	return 0;
}

int is_unary(char c)
{
	if ((c >= 35 && c <= 38) || (c == 33)) return (1 << 5);
	return 0;
}

int is_binary(char c)
{
	if (c == 42 || c == 43 || c == 45 
	 		 || c == 46 || c == 47 || c == 60
			 || c == 61 || c == 62) return (1 << 6);
	return 0;
}
