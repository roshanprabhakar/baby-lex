#include <stdlib.h>

#include "bitmap.h"

void init_bitmap(struct bitmap *map, int n)
{
	map->mem = malloc((n + 7) / 8);
}

void destroy_bitmap(struct bitmap *map)
{
	free(map->mem);
}

int bitmap_query(struct bitmap *map, int n)
{
	return ((map->mem[n / 8]) & (1 << (n % 8))) ? 1 : 0;
}

int bitmap_set(struct bitmap *map, int n)
{
	int ret = bitmap_query(map, n);
	map->mem[n / 8] |= (1 << n % 8);
	return ret;
}

