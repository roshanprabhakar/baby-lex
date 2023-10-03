#include <stdio.h>

#include "state.h"

void init_state(struct state *s)
{
	for (int i = 0; i < sizeof(s->group_connections) / sizeof(struct queue); ++i)
	{
		init_queue(s->group_connections + i, sizeof(struct state *), 2);
	}
	init_queue(&s->nil_connections, sizeof(struct state *), 5);
	init_queue(&s->char_queues, sizeof(struct char_queue), 5);
}

static void destroy_char_queue(void *p)
{
	struct char_queue *q = (struct char_queue *)p;
	destroy_queue(&q->q);
}

void destroy_state(struct state *s)
{
	for (int i = 0; i < sizeof(s->group_connections) / sizeof(struct queue); ++i)
	{
		destroy_queue(s->group_connections + i);
	}
	destroy_queue(&s->nil_connections);
	for_each(&s->char_queues, &destroy_char_queue);
	destroy_queue(&s->char_queues);
}

static void dump_state_queue_entry(void *p)
{
	printf("%d ", (*((struct state **)p))->id);
}

static void dump_state_char_queue(void *p)
{
	struct char_queue *cq = (struct char_queue *)p;
	printf("[%c: ", cq->c);
	for_each(&cq->q, &dump_state_queue_entry);
	printf("]");
}

/* This function takes a double pointer to a state, and is meant for
 * inspecting the content of queues (typically pointers). */
void dump_state(void *p)
{
	struct state *s = *(struct state **)p;
	printf("------- STATE %d -------\n", s->id);
	for (int i = 0; i < sizeof(s->group_connections) / sizeof(struct queue); ++i)
	{
		struct queue *q = s->group_connections + i;
		printf("%ld group %c connections: ", queue_length(q), 'A' + i);
		for_each(q, &dump_state_queue_entry);
		printf("\n");
	}
	printf("char queues: ");
	for_each(&s->char_queues, &dump_state_char_queue);
	printf("\n");

	struct queue *q = &s->nil_connections;
	printf("%ld nil connections: ", queue_length(q));
	for_each(q, &dump_state_queue_entry);
	printf("\n");
}

struct char_queue *state_get_char_queue(struct state *s, char c)
{
	struct queue char_queue_dup = s->char_queues;
	for (int num_queues = queue_length(&char_queue_dup);
			 num_queues;
			 --num_queues)
	{
		struct char_queue cq;
		queue_peek(&char_queue_dup, &cq);
		if (cq.c == c)
		{
			return (struct char_queue *) s->char_queues.data +
				s->char_queues.pop_curs;
		}
		queue_pop(&char_queue_dup, &cq);
	}
	return 0x0;
}

void state_insert_char_queue(struct state *s, char c)
{
	struct char_queue new_cq;
	new_cq.c = c;
	init_queue(&new_cq.q, sizeof(struct state *), 2);
	queue_push(&s->char_queues, &new_cq);
}

