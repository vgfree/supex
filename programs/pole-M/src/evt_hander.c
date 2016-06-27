#include "evt_hander.h"
#include "evt_worker.h"
#include "libmini.h"
#include "pole_common.h"

#include <assert.h>

static client_info_t *new_regist_one_client(xmq_ctx_t *xmq_ctx, evt_ctx_t *evt_ctx, hashmap_t *hmap, char id[IDENTITY_SIZE])
{
	client_info_t *p_info = NULL;

	/* Create master's client infor state.*/
	p_info = client_info_create(id);
	assert(p_info != NULL);

	p_info->xmq_ctx = xmq_ctx;
	p_info->evt_ctx = evt_ctx;
	p_info->hmap = hmap;
	client_info_init(p_info);

	p_info->consumer = xmq_get_consumer(xmq_ctx, id);
	if (!p_info->consumer) {
		x_printf(I, "xmq_get_consumer: fail! This is the first time to create '%s' XMQ Consumer.", id);

		if (xmq_register_consumer(xmq_ctx, id)) {
			x_printf(E, "xmq_register_consumer: Execute fail. Client Id <%s>.", id);
			return NULL;
		}

		p_info->consumer = xmq_get_consumer(xmq_ctx, id);
		assert(p_info->consumer != NULL);
		x_printf(I, "xmq_register_consumer: Execute succeed and get consumer okay! ");
	}
	return p_info;
}


int push_event_to_work(xmq_ctx_t *xmq_ctx, evt_ctx_t *evt_ctx, tlpool_t *tlpool, hashmap_t *hmap, int threads, evt_t *evt)
{
	if (!evt) {
		return -1;
	}

	/* hashmap_get()内部对vlen的值做了筛选,所以这里必须设置一个值 */
	client_info_t *p_info = NULL;
	size_t          vlen = sizeof(p_info);

	bool ok = hashmap_get(hmap, (void *)evt->id, strlen(evt->id), (void *)&p_info, &vlen);
	if (!ok || !p_info) {
		if (evt->ev_type == NET_EV_DUMP_REQ) {
			/* 交换ID */
			x_printf(D, "====Before: evt->id=[%s] evt->ev_data=[%s].\n", evt->id, evt->ev_data);

			if (strlen(evt->ev_data) >= strlen(evt->id)) {
				char buf[IDENTITY_SIZE] = { 0 };
				strcpy(buf, evt->id);
				strcpy(evt->id, evt->ev_data);
				strcpy(evt->ev_data, buf);
			} else {
				int ev_len = strlen(evt->id) + 1;
				evt_t *ev_new = evt_new_by_size(ev_len);
				assert(ev_new);
				memcpy(ev_new, evt, evt_head_size());
				strcpy(ev_new->id, evt->ev_data);
				strcpy(ev_new->ev_data, evt->id);
				ev_new->ev_size = ev_len;

				free_evt(evt);
				evt = ev_new;
			}

			x_printf(D, "====After : evt->id=[%s] evt->ev_data=[%s].\n", evt->id, evt->ev_data);
			
			evt->ev_type = NET_EV_DUMP_REP;
			evt->ev_state = NET_EV_FAIL;
			x_printf(E, "SERVER->DO_DUMP_REQ: Execute fail. Destination-Client no startup.");
			assert(0 == send_evt(evt_ctx, evt));
			return -1;
		}
		p_info = new_regist_one_client(xmq_ctx, evt_ctx, hmap, evt->id);

		hashmap_set(hmap, (void *)&evt->id, strlen(evt->id), (void *)&p_info, sizeof(p_info));



		/* push task */
		int hash = custom_hash(evt->id, threads, 0);
		x_printf(D, "PUSH_EVENT_TO_THREAD: Thread<%d>....\n", hash);

		struct online_task task;
		task.addr = p_info;
		ok = tlpool_push(tlpool, &task, TLPOOL_TASK_ALONE, hash);
		assert(ok);
	}
	if (evt->ev_type == NET_EV_DUMP_REQ) {
		char buf[IDENTITY_SIZE] = { 0 };
		strcpy(buf, evt->ev_data);
		
		client_info_t *p_temp = NULL;
		vlen = sizeof(p_temp);
		
		ok = hashmap_get(hmap, (void *)buf, strlen(buf), (void *)&p_temp, &vlen);
		if (!ok || !p_temp) {
			p_temp = new_regist_one_client(xmq_ctx, evt_ctx, hmap, buf);

			hashmap_set(hmap, (void *)&buf, strlen(buf), (void *)&p_temp, sizeof(p_temp));
		}
	}

	/* 添加新的事件到事件链表,供协程处理; */
	QITEM *item = qitem_init(NULL);
	assert(item);
	item->data = evt;

	if (evt->ev_type == NET_EV_DUMP_REQ || evt->ev_type == NET_EV_DUMP_REP) {
		qlist_push(&p_info->qdump, item);
	} else {
		qlist_push(&p_info->qevts, item);
	}
	return 0;
}


void event_dispenser_startup(xmq_ctx_t *xmq_ctx, evt_ctx_t *evt_ctx, tlpool_t *tlpool, hashmap_t *hmap, int threads)
{
	x_printf(I, "Event dispenser has startup...");

	while (1) {
		evt_t *evt = recv_evt(evt_ctx);
		if (!evt) {
			usleep(1000);
			continue;
		}
		x_printf(D, "Dispenser New Event:\n");
		print_evt(evt);

		/* 如果当前的事件类型为DUMP_REQ,那么交换id和ev_data的位置.*/
		if (evt->ev_type == NET_EV_DUMP_REQ) {
			if ((evt->ev_size == 0) || (evt->ev_size >= IDENTITY_SIZE)
					|| (strlen(evt->ev_data) == 0) || (strlen(evt->ev_data) >= IDENTITY_SIZE)) {
				x_printf(E, "The event of type [DUMP_REQ]'s target "
						"node ID is null or length >= IDENTITY_SIZE(32). We'll do nothing!");
				free_evt(evt);
				continue;
			}

			x_printf(D, "====Before: evt->id=[%s] evt->ev_data=[%s].\n", evt->id, evt->ev_data);

			if (strlen(evt->ev_data) >= strlen(evt->id)) {
				char buf[IDENTITY_SIZE] = { 0 };
				strcpy(buf, evt->id);
				strcpy(evt->id, evt->ev_data);
				strcpy(evt->ev_data, buf);
			} else {
				int ev_len = strlen(evt->id) + 1;
				evt_t *ev_new = evt_new_by_size(ev_len);
				assert(ev_new);
				memcpy(ev_new, evt, evt_head_size());
				strcpy(ev_new->id, evt->ev_data);
				strcpy(ev_new->ev_data, evt->id);
				ev_new->ev_size = ev_len;

				free_evt(evt);
				evt = ev_new;
			}

			x_printf(D, "====After : evt->id=[%s] evt->ev_data=[%s].\n", evt->id, evt->ev_data);
		}

		/* 根据事件不同的客户端标识,通过求余算法,将每个事件放入不同的处理线程 */
		push_event_to_work(xmq_ctx, evt_ctx, tlpool, hmap, threads, evt);
	}
}
