#ifndef QUEUE_H
#define QUEUE_H

// We need to be able to convert a struct entry into a queue we can push and pop from.
struct queue
{
	unsigned long push_curs;
	unsigned long pop_curs;
	unsigned long capacity;

	unsigned long atom_size_bytes;
	void *data;
};

int init_queue(struct queue *q, unsigned long atom_size_bytes, unsigned long capacity);
void destroy_queue(struct queue *q);

int queue_push(struct queue *q, void *src);
int queue_pop(struct queue *q, void *dst);
void *queue_peek(struct queue *q);

#endif // QUEUE_H
