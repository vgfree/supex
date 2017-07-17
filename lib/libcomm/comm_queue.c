/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/17.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_queue.h"

bool commqueue_init(struct comm_queue *list, unsigned int dsz, unsigned int capacity)
{
	assert(list && dsz > 0);
	assert(capacity >= 1);

	memset(list, 0, sizeof(struct comm_queue));
	list->capacity = capacity;
#if 0
	list->all = capacity + 1;
#else
	list->all = capacity / 256 + 1 > 1 ?  capacity / 256 + 1 : 2;	// must > 1
#endif
	list->dsz = dsz;
	list->isz = 0;
	list->osz = 0;
	list->nodes = 0;
	/*have one can't to use*/
	list->slots = calloc(list->all, dsz);
	assert(list->slots);

	list->headidx = 0;
	list->tailidx = 0;

	pthread_spin_init(&list->w_lock, 0);
	pthread_spin_init(&list->r_lock, 0);

	if (list->slots) {
		list->init = true;
		return true;
	} else {
		list->init = false;
		return false;
	}
}

static void _expand_queue(struct comm_queue *list)
{
	assert(list);
	unsigned int swap_all =
		(list->all - 1) * 2 < list->capacity ? (list->all - 1) * 2 + 1 : list->capacity + 1;
	void *swap_slots = calloc(swap_all, list->dsz);
	assert(swap_slots);

	if (list->tailidx >= list->headidx) {
		memcpy(swap_slots, &((char *)list->slots)[list->headidx * list->dsz], (list->tailidx - list->headidx) * list->dsz);
		list->tailidx = list->tailidx - list->headidx;
	} else {
		memcpy(swap_slots, &((char *)list->slots)[list->headidx * list->dsz], (list->all - list->headidx) * list->dsz);
		memcpy(swap_slots + (list->all - list->headidx) * list->dsz,
			&((char *)list->slots)[0], list->tailidx * list->dsz);
		list->tailidx = list->all - list->headidx + list->tailidx;
	}

	list->headidx = 0;
	free(list->slots);
	list->slots = swap_slots;
	list->all = swap_all;
}

/*free lock is just read and write don't need lock*/
bool commqueue_push(struct comm_queue *list, void *data)
{
	bool ok = false;

	assert(list && list->init && data);

	pthread_spin_lock(&list->w_lock);

	unsigned int next = (list->tailidx + 1) % list->all;

	if (list->headidx != next) {
		memcpy(&((char *)list->slots)[list->tailidx * list->dsz], (char *)data, list->dsz);
		list->tailidx = next;
		list->isz++;
		ok = true;
		AO_INC(&list->nodes);
	} else {
		if (list->all <= list->capacity) {
			pthread_spin_lock(&list->r_lock);
			_expand_queue(list);
			memcpy(&((char *)list->slots)[list->tailidx * list->dsz], (char *)data, list->dsz);
			list->tailidx++;
			list->isz++;
			ok = true;
			AO_INC(&list->nodes);
			pthread_spin_unlock(&list->r_lock);
		}
	}

	pthread_spin_unlock(&list->w_lock);

	return ok;
}

bool commqueue_pull(struct comm_queue *list, void *data)
{
	bool ok = false;

	assert(list && list->init && data);

	pthread_spin_lock(&list->r_lock);

	if (list->headidx != list->tailidx) {
		memcpy(data, &((char *)list->slots)[list->headidx * list->dsz], list->dsz);
		list->headidx = (list->headidx + 1) % list->all;
		list->osz++;
		ok = true;
		AO_DEC(&list->nodes);
	}

	pthread_spin_unlock(&list->r_lock);

	return ok;
}

bool commqueue_top(struct comm_queue *list, void *data)
{
	bool ok = false;

	assert(list && data);

	pthread_spin_lock(&list->r_lock);

	if (list->headidx != list->tailidx) {
		memcpy(data, &((char *)list->slots)[list->headidx * list->dsz], list->dsz);
		ok = true;
	}

	pthread_spin_unlock(&list->r_lock);

	return ok;
}

void commqueue_travel(struct comm_queue *list, bool (*travel)(void *, size_t size, void *), void *data)
{
	unsigned int    headidx = 0;
	unsigned int    tailidx = 0;

	assert(list && travel);

	pthread_spin_lock(&list->r_lock);

	headidx = list->headidx;
	tailidx = list->tailidx;

	while (headidx != tailidx) {
		bool ret = false;
		ret = travel(&((char *)list->slots)[headidx * list->dsz], list->dsz, data);

		if (!ret) {
			break;
		}

		headidx = (headidx + 1) % list->all;
	}

	pthread_spin_unlock(&list->r_lock);
}

void commqueue_destroy(struct comm_queue *list, TRAVEL_FCB fcb, void *usr)
{
	assert(list);
	int idx = 0;

	if (list->init) {
		pthread_spin_lock(&list->w_lock);
		pthread_spin_lock(&list->r_lock);

		if (list->slots) {
			if ((list->nodes > 0) && fcb) {
				while (list->headidx != list->tailidx) {
					char *data = &((char *)list->slots)[list->headidx * list->dsz];
					fcb(data, list->dsz, idx++, usr);

					list->headidx = (list->headidx + 1) % list->all;
					list->osz++;
					AO_DEC(&list->nodes);
				}
			}

			free(list->slots);
			list->slots = NULL;
		}

		list->init = false;
		pthread_spin_unlock(&list->r_lock);
		pthread_spin_unlock(&list->w_lock);
	}
}

