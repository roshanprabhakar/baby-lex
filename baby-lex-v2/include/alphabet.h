#ifndef BABYLEX_ALPHABET_H
#define BABYLEX_ALPHABET_H

/* We need a function that takes as input a character, and outputs
 * all the different queues the character belongs to. There are 4 
 * groups a character may be a part of: lower or upper letters, letters,
 * or digits. A character can trigger 0, 1, or more of these groups.
 * This character should also be given a dedicated queue in the 
 * associated automaton state. 
 */

#define CHAR_GROUP_MASK_LOWER 		(1 << 0)
#define CHAR_GROUP_MASK_UPPER 		(1 << 1)
#define CHAR_GROUP_MASK_LETTER 		(CHAR_GROUP_MASK_LOWER | CHAR_GROUP_MASK_UPPER)
#define CHAR_GROUP_MASK_DIGIT			(1 << 2)

int get_groups(char c)
{
	int ret = 0;
	// TODO
	return ret;
}

#endif // BABYLEX_ALPHABET_H
