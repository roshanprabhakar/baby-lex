#include "state_set.h"

void init_state_set(struct state_set *set, int num_states)
{
	init_queue(&set->states, sizeof(struct state *), num_states);
	init_bitmap(&set->contained_ids, num_states);
}

void destroy_state_set(struct state_set *set)
{
	destroy_queue(&set->states);
	destroy_bitmap(&set->contained_ids);
}

void state_set_add_state(struct state_set *set, struct state *s)
{
	if (!bitmap_query(&set->contained_ids, s->id))
	{
		bitmap_set(&set->contained_ids, s->id);
		queue_push(&set->states, &s);
	}
}

int state_set_contains_state(struct state_set *set, struct state *s)
{
	return bitmap_query(&set->contained_ids, s->id);
}

