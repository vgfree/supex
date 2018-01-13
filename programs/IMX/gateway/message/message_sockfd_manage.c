#include "message_sockfd_manage.h"

#define NODE_SIZE 10240

struct core_exchange_node
{
	int     fd_array[NODE_SIZE];
	int     max_size;
	pthread_rwlock_t rwlock;
};
static struct core_exchange_node g_sockfd_manage = {.rwlock = PTHREAD_RWLOCK_INITIALIZER};

void message_sockfd_manage_add(int sockfd)
{
	pthread_rwlock_wrlock(&g_sockfd_manage->rwlock);
	if (g_sockfd_manage.max_size > NODE_SIZE) {
		x_printf(E, "core exchange node:%d > max size:%d.", g_sockfd_manage.max_size, NODE_SIZE);
		abort();
	}

	g_sockfd_manage.fd_array[g_sockfd_manage.max_size++] = sockfd;
	pthread_rwlock_unlock(&g_sockfd_manage->rwlock);
}

void message_sockfd_manage_del(int sockfd)
{
	pthread_rwlock_wrlock(&g_sockfd_manage->rwlock);
	for (int i = 0; i < g_sockfd_manage.max_size; i++) {
		if (g_sockfd_manage.fd_array[i] == sockfd) {
			memcpy(&g_sockfd_manage.fd_array[i], &g_sockfd_manage.fd_array[ i + 1], g_sockfd_manage.max_size - i -1);
			g_sockfd_manage.max_size--;
			break;
		}
	}
	pthread_rwlock_unlock(&g_sockfd_manage->rwlock);
}

int message_sockfd_manage_travel(MANAGE_TRAVEL_FOR_LOOKUP_FCB lfcb, MANAGE_TRAVEL_FOR_DELETE_FCB dfcb, void *usr)
{
	if (dfcb) {
		pthread_rwlock_wrlock(&g_sockfd_manage->rwlock);
	} else {
		pthread_rwlock_rdlock(&g_sockfd_manage->rwlock);
	}
	int i = 0;
	for (; i < g_sockfd_manage.max_size; i++) {
		if (lfcb) {
			lfcb(g_sockfd_manage.fd_array[i], i, usr);
		}
		if (dfcb) {
			bool discard = dfcb(g_sockfd_manage.fd_array[i], i, usr);
			if (discard) {
				memcpy(&g_sockfd_manage.fd_array[i], &g_sockfd_manage.fd_array[ i + 1], g_sockfd_manage.max_size - i -1);
				g_sockfd_manage.max_size--;
				i--;
			}
		}
	}
	pthread_rwlock_unlock(&g_sockfd_manage->rwlock);
	return i;
}
