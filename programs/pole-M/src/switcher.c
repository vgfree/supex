#include <string.h>
#include <pthread.h>

#include "pipe.h"
#include "porter.h"
#include "slog/slog.h"

#include "netmod.h"

static porter_info_t *__porter_build(event_ctx_t *ev_ctx, xmq_ctx_t *ctx, const char *client_id)
{
	int             res = 0;
	xmq_consumer_t  *consumer = xmq_get_consumer(ctx, client_id);

	if (!consumer) {
		x_printf(I, "xmq_get_consumer(%s) fail, The consumer name doesn't exist.", client_id);

		res = xmq_register_consumer(ctx, client_id);

		if (res != 0) {
			x_printf(W, "xmq_register_consumer(%s) fail.", client_id);
			return NULL;
		}

		consumer = xmq_get_consumer(ctx, client_id);
	}

	porter_info_t *porter = porter_create(client_id, consumer, ev_ctx, xmq_fetch_position(consumer));

	if (!porter) {
		x_printf(E, "porter_create failed");
		xmq_unregister_consumer(ctx, client_id);
		return NULL;
	}

	pthread_mutex_lock(&porters_list_lock);
	list_add_tail(&porter->list, &porters_list_head);
	pthread_mutex_unlock(&porters_list_lock);

	res = porter_work_start(porter);

	if (res) {
		x_printf(E, "porter_work_start failed");

		list_del(&porter->list);
		xmq_unregister_consumer(ctx, client_id);
		porter_destroy(porter);

		return NULL;
	}

	return porter;
}

extern int switcher_work(event_ctx_t *ev_ctx, xmq_ctx_t *ctx)
{
	event_t *event = NULL;

	/* Receving All the Clients's event requests, and push them into their pipes. */
	while ((event = recv_event(ev_ctx, -1))) {
		printf("SWITCHER: RECV_EVENT %s.\n", _systime());
		x_printf(D, "switcher_work recv event:");	// test
		print_event(event);				// test

		/* Only for the first time to create master's porter. */
		pthread_mutex_lock(&porters_list_lock);

		if (list_empty(&porters_list_head)) {
			pthread_mutex_unlock(&porters_list_lock);

			if (event->ev_type == NET_EV_INCREMENT_REQ) {
				if (strncmp(event->id, "master", strlen("master") + 1)) {
					x_printf(W, "recv event with id [%s] is not [master]", event->id);
					delete_event(event);
					continue;
				}

				porter_info_t *porter = __porter_build(ev_ctx, ctx, event->id);

				if (!porter) {
					x_printf(E, "first_porter_build failed");
					delete_event(event);
					continue;
				}

				pipe_push(&porter->events, event);
			}

			continue;
		} else {
			pthread_mutex_unlock(&porters_list_lock);
		}

		porter_info_t *porter = NULL;

		switch (event->ev_type)
		{
			case NET_EV_DUMP_REQ:

				if (strlen(event->ev_data) >= sizeof(event->id)) {
					x_printf(W, "name of porter [%s] is too long [%lu]", event->ev_data, strlen(event->ev_data));
					delete_event(event);
					continue;
				}

				porter = look_up_porter(event->ev_data);
				break;

			case NET_EV_DUMP_REP:
				porter = look_up_porter(event->id);
				break;

			case NET_EV_INCREMENT_REQ:
				porter = look_up_porter(event->id);
				break;

			default:
				x_printf(W, "invalid event with type [%d]", event->ev_type);
		}	// end switch

		if (porter) {
			if (porter->tid == 0) {
				int res = porter_work_start(porter);

				if (res) {
					x_printf(E, "porter [%s] start faild", porter->peer);
					delete_event(event);
					continue;
				}
			}

			pipe_push(&porter->events, event);
		} else {
			x_printf(W, "no correponding porter with this peer[%s]", event->id);
			delete_event(event);
		}
	}	// end while.

	return -1;
}

extern void switcher_join_all_porter(void)
{
	list_t          *node = NULL;
	porter_info_t   *porter = NULL;

	pthread_mutex_lock(&porters_list_lock);

	list_foreach(node, &porters_list_head)
	{
		porter = container_of(node, porter_info_t, list);

		if (porter->tid) {
			pthread_join(porter->tid, NULL);
		}
	}
	list_del_all_entrys(&porters_list_head, porter_info_t, list);

	pthread_mutex_unlock(&porters_list_lock);
}

