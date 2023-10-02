#ifndef ARRBUF_H
#define ARRBUF_H

#define ARR_BUF_REALLOC_DELTA 100

// Buffer where data is read but never cleared.
struct buffer
{
	void *p;
	long write_curs;
	long capacity;
};

// Returns the location at which the data was just copied into.
// Returns null on failure.
void *buffer_insert(struct buffer *b, void *data, long nbytes);

// Returns the write cursor, and advances the write cursor by
// nbytes.
void *buffer_alloc(struct buffer *b, long nbytes);

#endif // ARRBUF_H
