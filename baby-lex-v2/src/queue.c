#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

#include "automaton.h"

#define QUEUE_CAPACITY_DELTA 3

int init_queue(struct queue *q,
								unsigned long atom_size_bytes,
								unsigned long capacity)
{
	if (!capacity || !atom_size_bytes)
	{
		printf("ERR: failed to construct queue.\n");
		return -1;
	}

	q->push_curs = q->pop_curs = 0;
	q->atom_size_bytes = atom_size_bytes;
	q->capacity = capacity;

	q->data = malloc(atom_size_bytes * capacity);
	if (!q->data) return -1;

	return 0;
}

void destroy_queue(struct queue *q)
{ free(q->data); }

int queue_peek(struct queue *q, void *dst)
{
	if (q->push_curs == q->pop_curs) return -1;
	memcpy(dst, (char *)q->data + q->pop_curs * q->atom_size_bytes, q->atom_size_bytes);
	return 0;
}

long queue_length(struct queue *q)
{
	return (q->push_curs < q->pop_curs) ?
		q->push_curs + q->capacity - q->pop_curs :
		q->push_curs - q->pop_curs;
}

// If the queue is empty, return -1. On success, dest holds the 
// first-in value, and 0 is returned.
int queue_pop(struct queue *q, void *dest)
{
	if (q->push_curs == q->pop_curs) return -1;

	memcpy(dest, (char *) q->data + q->pop_curs * q->atom_size_bytes, q->atom_size_bytes);

	++(q->pop_curs);
	if (q->pop_curs == q->capacity) q->pop_curs = 0;

	return 0;
}

// Assumes that if the write cursor equals the read cursor,
// then the queue is full, NOT empty.
static int queue_extend(struct queue *q)
{

	if (q->push_curs == q->pop_curs)
	{
		void *new_buf = malloc(
				q->atom_size_bytes * (q->capacity + QUEUE_CAPACITY_DELTA)
		);
		if (!new_buf) return -1;

		unsigned long post = q->atom_size_bytes * (q->capacity - q->pop_curs);
		unsigned long pre = q->atom_size_bytes * (q->push_curs);

		memcpy(new_buf, (char *) q->data + pre, post);
		memcpy((char *) new_buf + post, q->data, pre);

		q->pop_curs = 0;
		q->push_curs = q->capacity;
		q->capacity += QUEUE_CAPACITY_DELTA;

		free(q->data); q->data = new_buf;
	}

	return 0;
}

// Assumes at least one slot available. On success, return 0.
int queue_push(struct queue *q, void *src)
{
	// Push onto the current slot.
	memcpy((char *) q->data + q->atom_size_bytes * q->push_curs, src, q->atom_size_bytes);
	++(q->push_curs);

	if ((char *) q->data + q->atom_size_bytes * q->push_curs ==
			(char *) q->data + q->atom_size_bytes * q->capacity)
	{
		q->push_curs = 0;
	}

	return queue_extend(q);
}

// fn is a function which accepts a pointer to objects of the 
// type contained q.
void for_each(struct queue *q, void (*fn)(void *))
{
	for (long curs = q->pop_curs; curs != q->push_curs; ++curs)
	{
		fn((char *)q->data + curs * q->atom_size_bytes);
		if (curs == q->capacity) curs = 0;
	}
}
