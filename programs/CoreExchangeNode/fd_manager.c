#include <assert.h>

#include "fd_manager.h"
#include "libmini.h"

static struct fd_list   g_list;
static struct fd_array  g_array;

int fdman_list_init(void)
{
	for (int i = 0; i < FD_MAX_CLASSIFICATION; i++) {
		g_list.head[i].size = 0;
		g_list.head[i].next = NULL;
	}

	return 0;
}

int fdman_list_destroy(void)
{
	for (int i = 0; i < FD_MAX_CLASSIFICATION; i++) {
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

int fdman_list_remove(const enum router_object        obj,
	const int                               fd)
{
	struct fd_node **curr = &g_list.head[obj].next;

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

int fdman_list_push_back(const enum router_object obj, const struct fd_node *node)
{
	struct fd_node *curr = g_list.head[obj].next;
	if (curr == NULL) {
		g_list.head[obj].next = (struct fd_node *)malloc(sizeof(struct fd_node));
		g_list.head[obj].next->fd = node->fd;
		g_list.head[obj].next->status = node->status;
		g_list.head[obj].next->next = NULL;
	} else {
		while (curr->next) {
			curr = curr->next;
		}
		curr->next = (struct fd_node *)malloc(sizeof(struct fd_node));
		curr->next->fd = node->fd;
		curr->next->status = node->status;
		curr->next->next = NULL;
	}
	return 0;
}

int fdman_list_front(const enum router_object obj,
	struct fd_node                  *node)
{
	if (g_list.head[obj].next) {
		*node = *g_list.head[obj].next;
		return 0;
	}

	return -1;
}

int fdman_array_init(void)
{
	g_array.dsp_array = (struct fd_descriptor *)
		calloc(FD_CAPACITY, sizeof(struct fd_descriptor));
	assert(g_array.dsp_array);
	g_array.max_fd = 0;
	g_array.cap = FD_CAPACITY;

	return 0;
}

int fdman_array_destroy()
{
	if (g_array.dsp_array == NULL) {
		return -1;
	}

	free(g_array.dsp_array);
	g_array.max_fd = 0;
	g_array.cap = 0;
	return 0;
}

int fdman_array_fill_fd(const int fd, const struct fd_descriptor *des)
{
	if (fd > g_array.cap) {
		x_printf(E, "fd array not enough capacity.");
		assert(0);
	}

	assert(g_array.dsp_array);

	memcpy(&(g_array.dsp_array[fd]), des, sizeof(struct fd_descriptor));

	if (g_array.max_fd < fd) {
		g_array.max_fd = fd;
	} else {
		x_printf(W, "max_fd >fd");
	}

	return 0;
}

int fdman_array_remove_fd(const int fd)
{
	g_array.dsp_array[fd].status = 2;
	return 0;
}

int fdman_array_at_fd(const int fd, struct fd_descriptor *des)
{
	assert(des);
	des->host = g_array.dsp_array[fd].host;
	des->port = g_array.dsp_array[fd].port;
	des->status = g_array.dsp_array[fd].status;
	des->obj = g_array.dsp_array[fd].obj;

	return 0;
}

uint32_t fdman_statistic_object(const enum router_object obj)
{
	uint32_t count = 0;

	switch (obj)
	{
		case CLIENT:
		{
			for (int i = 0; i < g_array.max_fd; i++) {
				if ((g_array.dsp_array[i].obj == CLIENT) &&
					(g_array.dsp_array[i].status == 1)) {
					count++;
				}
			}

			break;
		}

		case MESSAGE_GATEWAY:
		{
			struct fd_node *node = g_list.head[MESSAGE_GATEWAY].next;

			while (node) {
				count++;
				node = node->next;
			}

			break;
		}

		case ROUTER_SERVER:
			printf("Not support statistc router server.");
			break;

		default:
			printf("statistic error.");
			break;
	}
	return count;
}

uint32_t fdman_poll_client_fd(int *arr[], uint32_t *size)
{
	uint32_t count = 0;

	for (int i = 0; i < g_array.max_fd; i++) {
		if ((g_array.dsp_array[i].obj == CLIENT) &&
			(g_array.dsp_array[i].status == 1)) {
			(*arr)[count] = i;
			count++;
		}
	}

	*size = count;
	return count;
}

