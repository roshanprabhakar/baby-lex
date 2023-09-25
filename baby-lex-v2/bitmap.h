#ifndef BITMAP_H
#define BITMAP_H

struct bitmap
{
	unsigned char *mem;
};

void init_bitmap(struct bitmap *map, int n);
void destroy_bitmap(struct bitmap *map);

int bitmap_query(struct bitmap *map, int n);
int bitmap_set(struct bitmap *map, int n);

#endif // BITMAP_H
