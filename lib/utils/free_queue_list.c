#include "free_queue_list.h"

void free_queue_init(struct free_queue_list *list, unsigned int dsz, unsigned int max)
{
	assert(max >= 1);
	memset(list, 0, sizeof(struct free_queue_list));
	list->max = max;
	list->all = max / 256 + 1 > 1 ?  max / 256 + 1 : 2; //must > 1
	list->dsz = dsz;
	list->isz = 0;
	list->osz = 0;
	/*have one can't to use*/
	list->slots = calloc(list->all, dsz);
	assert(list->slots);
	list->head = 0;
	list->tail = 0;
	AO_SpinLockInit(&list->w_lock, false);
	AO_SpinLockInit(&list->r_lock, false);
}

static void _expand_queue(struct free_queue_list *list)
{
	assert(list); 
	unsigned int swap_all =
		(list->all - 1) * 2 < list->max ? (list->all - 1) * 2 + 1 : list->max + 1;
	void *swap_slots = calloc(swap_all, list->dsz);
	assert(swap_slots);
	if (list->tail >= list->head) {
		memcpy(swap_slots, &((char *)list->slots)[list->head * list->dsz], (list->tail - list->head) * list->dsz);
		list->tail = list->tail - list->head;
	}
	else {
		memcpy(swap_slots, &((char *)list->slots)[list->head *list->dsz], (list->all - list->head) * list->dsz);
		memcpy(swap_slots + (list->all - list->head) *list->dsz,
				&((char *)list->slots)[0], list->tail * list->dsz);
		list->tail = list->all - list->head + list->tail;
	}
	list->head = 0;
	free(list->slots);
	list->slots = swap_slots;
	list->all = swap_all;
}
/*free lock is just read and write don't need lock*/
bool free_queue_push(struct free_queue_list *list, void *data)
{
	bool ok = false;

	assert(list && data);

	AO_SpinLock(&list->w_lock);

	unsigned int next = (list->tail + 1) % list->all;

	if (list->head != next) {
		memcpy(&((char *)list->slots)[list->tail * list->dsz], (char *)data, list->dsz);
		list->tail = next;
		list->isz++;
		ok = true;
		ATOMIC_INC(&list->tasks);
	}
	else {
		if (list->all <= list->max) {
			AO_SpinLock(&list->r_lock);
			_expand_queue(list);
			memcpy(&((char *)list->slots)[list->tail * list->dsz], (char *)data, list->dsz);
			list->tail++;
			list->isz++;
			ok = true;
			ATOMIC_INC(&list->tasks);
			AO_SpinUnlock(&list->r_lock);
		}
	}
	AO_SpinUnlock(&list->w_lock);
	return ok;
}

bool free_queue_pull(struct free_queue_list *list, void *data)
{
	bool ok = false;
	assert(list && data);
	AO_SpinLock(&list->r_lock);
	if (list->head != list->tail) {
		memcpy(data, &((char *)list->slots)[list->head * list->dsz], list->dsz);
		list->head = (list->head + 1) % list->all;
		list->osz++;
		ok = true;
		ATOMIC_DEC(&list->tasks);
	}
	AO_SpinUnlock(&list->r_lock);
	return ok;
}
