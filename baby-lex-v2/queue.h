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

// The queue elements of src are modifiable from dst.
// IMPORTANT do NOT push elements into dst. The duplicate should only be used
// for iteration, and may be safely popped from.
void queue_dup(struct queue *dst, struct queue *src);

int queue_push(struct queue *q, void *src);
int queue_pop(struct queue *q, void *dst);
void *queue_peek(struct queue *q);

long queue_length(struct queue *q);

// For debug purposes.
void dump_queue(struct queue *q, void (*print_entry) (void *));

#endif // QUEUE_H
