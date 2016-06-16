#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>
#include <stdlib.h>

typedef void (*callback_t) (void *callback_data);

struct timer_item
{
	callback_t      callback;
	void            *user_data;
	uint64_t        interval;
	uint64_t        next_tick;
};

struct timer_list
{
	struct timer_item       *timer;
	int                     cap;
	int                     max_pos;
};

struct timer_list       *create_timer_list();

int destroy_timer_list(struct timer_list *list);

int add_timer(struct timer_list *list, struct timer_item *timer);

int remove_timer(struct timer_list *list, uint8_t id);

void schedule_timer(struct timer_list *list);
#endif	/* ifndef _TIMER_H_ */

