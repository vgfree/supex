#include <assert.h>

#include "libmini.h"
#include "exc_sockfd_manager.h"

static struct fd_list g_list;

int fdman_list_init(void)
{
	for (int i = 0; i < MAX_ROUTER_KIND; i++) {
		g_list.head[i].size = 0;
		g_list.head[i].next = NULL;
	}

	return 0;
}

int fdman_list_free(void)
{
	for (int i = 0; i < MAX_ROUTER_KIND; i++) {
		struct fd_node  *curr;
		struct fd_node  *next;
		curr = g_list.head[i].next;

		while (curr) {
			next = curr->next;
			free(curr);
			curr = next;
		}

		g_list.head[i].next = NULL;
	}

	return 0;
}

int fdman_list_del(const enum router_type type, const int fd)
{
	struct fd_node **curr = &g_list.head[type].next;

	while (*curr) {
		struct fd_node *entry = *curr;

		if (entry->fd == fd) {
			*curr = entry->next;
			free(entry);
			return 0;
		} else {
			curr = &entry->next;
		}
	}

	return -1;
}

int fdman_list_add(const enum router_type type, const struct fd_node *node)
{
	struct fd_node  *curr = g_list.head[type].next;
	struct fd_node  *news = (struct fd_node *)malloc(sizeof(struct fd_node));

	news->next = NULL;
	news->fd = node->fd;
	news->status = node->status;

	if (curr == NULL) {
		g_list.head[type].next = news;
	} else {
		while (curr->next) {
			curr = curr->next;
		}

		curr->next = news;
	}

	return 0;
}

int fdman_list_top(const enum router_type type, struct fd_node *node)
{
	struct fd_node *curr = g_list.head[type].next;

	if (curr) {
		*node = *curr;
		return 0;
	}

	return -1;
}

/*==========================================================*/

#include "exc_comm_def.h"
static struct fd_slot g_slot;

int fdman_slot_init(void)
{
	g_slot.info = (struct fd_info *)calloc(FD_CAPACITY, sizeof(struct fd_info));
	assert(g_slot.info);
	g_slot.max = 0;
	g_slot.cap = FD_CAPACITY;
	return 0;
}

int fdman_slot_free(void)
{
	assert(g_slot.info);
	free(g_slot.info);
	g_slot.info = NULL;
	g_slot.max = 0;
	g_slot.cap = 0;
	return 0;
}

int fdman_slot_set(const int fd, const struct fd_info *des)
{
	if (fd > g_slot.cap) {
		x_printf(E, "fd slot not enough capacity.");
		assert(0);
	}

	assert(g_slot.info);

	memcpy(&(g_slot.info[fd]), des, sizeof(struct fd_info));

	if (g_slot.max < fd) {
		g_slot.max = fd;
	} else {
		x_printf(W, "new fd small the last added!");
	}

	return 0;
}

int fdman_slot_del(const int fd)
{
	struct fd_info *des = &(g_slot.info[fd]);

	assert(des->status == 1);

	memset(des, 0, sizeof(struct fd_info));
	return 0;
}

int fdman_slot_get(const int fd, struct fd_info *des)
{
	assert(des);
	memcpy(des, &(g_slot.info[fd]), sizeof(struct fd_info));
	return 0;
}

