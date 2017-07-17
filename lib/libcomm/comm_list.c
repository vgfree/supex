#include "comm_list.h"

void commlist_init(struct comm_list *list)
{
	assert(list);
	list->tail = NULL;
	list->head = NULL;
	list->nodes = 0;
	pthread_spin_init(&list->lock, 0);
	list->init = true;
}

void commlist_destroy(struct comm_list *list, TRAVEL_FCB fcb, void *usr)
{
	assert(list);
	int idx = 0;

	if (list->init) {
		pthread_spin_lock(&list->lock);
		while (list->head) {
			struct list_node *node = list->head;
			list->head = list->head->next;
			if (fcb) {
				fcb(node->data, node->size, idx++, usr);
			}
			free(node);
			list->nodes--;
		}
		list->tail = NULL;
		list->init = false;
		pthread_spin_unlock(&list->lock);
	}
}


bool commlist_push(struct comm_list *list, void *data, size_t size)
{
	assert(list && data);

	struct list_node *node = calloc(1, sizeof(struct list_node) + size);
	if (!node) {
		return false;
	}
	node->prev = NULL;
	node->next = NULL;
	node->size = size;
	memcpy(node->data, data, size);

	pthread_spin_lock(&list->lock);
	node->prev = list->tail;
	if (list->head != NULL) {
		list->tail->next = node;
		list->tail = node;
	} else {
		list->head = node;
		list->tail = node;
	}

	list->nodes++;
	pthread_spin_unlock(&list->lock);
	return true;
}

bool commlist_pull(struct comm_list *list, void *data, size_t size)
{
	assert(list && data);

	pthread_spin_lock(&list->lock);
	if (list->nodes > 0) {
		struct list_node *node = list->head;

		if (list->head == list->tail) {
			/* 取的是最后一个节点数据 */
			list->tail = NULL;
		}
		list->head = list->head->next;

		list->nodes--;

		size_t todo = MIN(size, node->size);
		memcpy(data, node->data, todo);
		free(node);
		pthread_spin_unlock(&list->lock);
		return true;
	}
	pthread_spin_unlock(&list->lock);
	return false;
}
