#include <stdlib.h>
#include <string.h>

#include "arrbuf.h"

void *buffer_insert(struct buffer *b, void *data, long nbytes)
{
	if (nbytes > b->capacity - b->write_curs)
	{ 
		b->capacity += ARR_BUF_REALLOC_DELTA + nbytes;
		b->p = realloc(b->p, b->capacity);
	}

	if (b->p)
	{
		void *save = (void *) ((char *) b->p + b->write_curs);
		memcpy((char *) b->p + b->write_curs, data, nbytes);
		b->write_curs += nbytes;
		return save;
	}
	else
	{
		return 0x0;
	}
}

void *buffer_alloc(struct buffer *b, long nbytes)
{
	void *save = NULL;
	if (b->capacity - b->write_curs < nbytes) goto end;
	else 
	{
		save = (void*) ((char *) b->p + b->write_curs);
		b->write_curs += nbytes;
	}
end:
	return save;
}
