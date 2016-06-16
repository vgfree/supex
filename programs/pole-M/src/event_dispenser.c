#include "event_handler.h"
#include "event_dispenser.h"
#include "slog/slog.h"

#include <assert.h>

int event_dispenser_startup(event_ctx_t *ev_ctx, int threads)
{
	event_t *ev = NULL;

	x_printf(I, "Event dispenser has startup...");

	while ((ev = recv_event(ev_ctx, -1)) != NULL) {
		DBG_PRINT("Dispenser New Event:\n");
		print_event(ev);

		/* 如果当前的事件类型为DUMP_REQ,那么交换id和ev_data的位置.*/
		if (ev->ev_type == NET_EV_DUMP_REQ) {
			if ((ev->ev_size == 0) || (strlen(ev->ev_data) == 0) \
				|| (strlen(ev->ev_data) >= IDENTITY_SIZE)) {
				x_printf(E, "The event of type [DUMP_REQ]'s target "
					"node ID is null or length >= IDENTITY_SIZE(32). We'll do nothing!");
				delete_event(ev);
				continue;
			}

			DBG_PRINTS("====Before: ev->id=[%s] ev->ev_data=[%s].\n", ev->id, ev->ev_data);

			if (strlen(ev->ev_data) >= strlen(ev->id)) {
				char buf[IDENTITY_SIZE] = { 0 };
				strcpy(buf, ev->id);
				strcpy(ev->id, ev->ev_data);
				strcpy(ev->ev_data, buf);
			} else {
				event_t *ev_new = event_new_size(strlen(ev->id) + 1);
				memcpy(ev_new, ev, event_head_size());
				strcpy(ev_new->id, ev->ev_data);
				strcpy(ev_new->ev_data, ev->id);
				ev_new->ev_size = strlen(ev->id) + 1;

				delete_event(ev);
				ev = ev_new;
			}

			DBG_PRINTS("====After : ev->id=[%s] ev->ev_data=[%s].\n", ev->id, ev->ev_data);
		}

		/* 根据事件不同的客户端标识,通过求余算法,将每个事件放入不同的处理线程,并在内部有协程处理*/
		push_event_to_thread(ev, get_thread_id_by(threads, ev->id));
	}

	return 0;
}

int get_thread_id_by(int threads, const char *id)
{
	assert(threads > 0 && id != NULL && strlen(id) > 0);

	int             ascii_summary = 0;
	const char      *p = id;

	for (; *p != '\0'; ++p) {
		ascii_summary += *p;
	}

	return ascii_summary % threads;		// 0 - (threads-1)
}

