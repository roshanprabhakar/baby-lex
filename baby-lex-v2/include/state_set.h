#ifndef NFA_STATE_SET_H
#define NFA_STATE_SET_H

#include "queue.h"
#include "bitmap.h"
#include "state.h"

struct state_set
{
	struct queue states;
	struct bitmap contained_ids;
};

void init_state_set(struct state_set *set, int num_states);
void destroy_state_set(struct state_set *set);
void state_set_add_state(struct state_set *set, struct state *s);
int state_set_contains_state(struct state_set *set, struct state *s);

#endif // NFA_STATE_SET_H
