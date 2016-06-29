#include "evt_hander.h"
#include "evt_worker.h"
#include "libmini.h"

#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

#include "core/evcs_module.h"
#include "core/evcs_events.h"
#include "core/evcs_kernel.h"
#include "spx_evcs.h"
#include "spx_evcs_module.h"

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, evcs_init, evcs_exit, &g_evcs_evts);

#include "_ev_coro.h"
#include "evcoro_async_tasks.h"
#include "thread_pool_loop/tlpool.h"

#include "conf.h"
extern struct pole_conf g_pole_conf;

#define EVENT_SEQ(evt) ((evt)->incr.task_seq)

#define EXEC_FLOW_ERROR(evt)								  \
	do {										  \
		x_printf(F, "Execution flow error: Never should be here. Process exit."); \
		print_evt(evt);								  \
		free_evt(evt); abort();							  \
	} while (0)

int do_incr_rep_task(client_info_t *clinfo)
{
	evt_t   *evt = NULL;
	QITEM   *item = qlist_pull(&clinfo->qevts);

	assert(item);
	evt = item->data;
	assert(evt);
	assert(evt->ev_type == NET_EV_INCREMENT_REQ);
	assert(EVENT_SEQ(evt) ||
		((evt->ev_state == NET_EV_NONE) && (EVENT_SEQ(evt) == 0)));

	/* 处理客户端重启时的各种情况 */
	if ((evt->ev_state == NET_EV_NONE) && (EVENT_SEQ(evt) == 0)) {
		/*
		 * 上次正在执行DUMP请求,可能已经发送出去了,没收到DUMP_REP,或者还有未处理的DUMP请求.
		 * 不管怎样,客户端肯定是宕掉了或被意外KILL的情况,具体原因不详!
		 * 长时间等待,请求DUMP的(其他)客户端,可自行断开和服务器的连接;
		 * 场景一: 已完成增量同步的相应.
		 *     不管上次是否成功发送 NET_EV_INCREMENT_REP，都将重发该事件
		 *     模式: A(C) -> A(S) -> A(C) C代表客户端; S代表服务器
		 * 场景二: 已发送完DUMP请求.
		 *     服务器传递DUMP请求给(执行DUMP操作的客户端A时,A客户端意外被KILL,或自己宕掉.
		 *     模式: B(C) -> A(S) -> A(C)
		 * 场景三: 第一次启动
		 *     节点在DUMP成功后首次启动, 或者master节点所对应的线程首次执行增量同步.
		 */

		/* 清理client_info*/
		client_info_init(clinfo);

		/* 设置时间的同步序列码 */
		evt->incr.task_seq = clinfo->consumer->last_fetch_seq;
	}
	/* 重启指点同步 */
	else if ((evt->ev_state == NET_EV_NONE) && (EVENT_SEQ(evt) != 0)) {
		/* 清理client_info*/
		client_info_init(clinfo);

		/* 修改数据库的值 */
		int res = xmq_update_that(clinfo->consumer, EVENT_SEQ(evt));
		assert(res == 0);

		/* 设置时间的同步序列码 */
		evt->incr.task_seq = clinfo->consumer->last_fetch_seq;
	}
	/* 正常同步 */
	else {
		/* 节点出现重大错误 */
		if (evt->ev_state == NET_EV_FATAL) {
			x_printf(F, "A fatal error[EV_FATAL] occured with client [%s]."
				" Will destroy the Client sync-state.", clinfo->id);
			free_evt(evt);
			qitem_free(item);
			return EVT_STATE_RUIN;
		}

		/* 节点出现一般性错误 */
		if (evt->ev_state == NET_EV_FAIL) {
			x_printf(F, "A generality error[EV_FAIL] occured with client [%s]."
				" we ignore it and stop sync!", clinfo->id);
			free_evt(evt);
			qitem_free(item);
			return EVT_STATE_EXIT;
		}

		/*
		 * 因为zmq链接断开，数据请求会暂存队列.
		 * 因此重启客户端会重复收到相同序列数据，以下条件会不成立.
		 * 需要客户端做保护
		 */
#if 1
		/* 检查当前的请求序列是不是上次的值加1. */
		assert(EVENT_SEQ(evt) == clinfo->consumer->last_fetch_seq + 1);
#endif

		/* 修改数据库的值 */
		int res = xmq_update_that(clinfo->consumer, EVENT_SEQ(evt));
		assert(res == 0);

		/* 设置时间的同步序列码 */
		evt->incr.task_seq = clinfo->consumer->last_fetch_seq;
	}

	/* 获取持久化队列中的消息，发送给对端节点. */
	evt->ev_state = NET_EV_SUCC;
	evt->ev_type = NET_EV_INCREMENT_REP;
	qlist_push(&clinfo->qincr, item);

	return EVT_STATE_WAIT;
}

int do_incr_chk_task(client_info_t *clinfo)
{
	evt_t   *evt = NULL;
	QITEM   *item = qlist_pull(&clinfo->qevts);

	assert(item);
	evt = item->data;
	assert(evt);
	assert(evt->ev_type == NET_EV_INCREMENT_REQ);

	if (evt->ev_state == NET_EV_NONE) {
		client_info_init(clinfo);
		/* 客户端重启 */
		qlist_push(&clinfo->qevts, item);
		return EVT_STATE_INCR;
	} else {
		/* 正常同步 */
		assert(false);
	}
}

int do_incr_req_task(client_info_t *clinfo)
{
	evt_t   *evt = NULL;
	QITEM   *item = qlist_pull(&clinfo->qincr);

	assert(item);
	evt = item->data;
	assert(evt);
	assert(evt->ev_type == NET_EV_INCREMENT_REP);

	uint64_t        task_seq = EVENT_SEQ(evt);
	xmq_msg_t       *msg = xmq_pull_that(clinfo->consumer, task_seq);	// FIXME:last_db_seq

	if (!msg) {
		/* 将未处理的事件,重新放回指定线程队列里,待下次处理. */
		qlist_push(&clinfo->qincr, item);
		return EVT_STATE_WAIT;
	} else {
		qitem_free(item);
		assert(xmq_msg_size(msg) > 0);

		/* 创建一个待发送的增量响应事件. */
		evt_t *ev_rsp = evt_new_by_size(xmq_msg_size(msg));
		assert(ev_rsp);

		memcpy(ev_rsp, evt, evt_head_size());
		ev_rsp->ev_size = xmq_msg_size(msg);
		ev_rsp->ev_type = NET_EV_INCREMENT_REP;
		memcpy(ev_rsp->ev_data, xmq_msg_data(msg), xmq_msg_size(msg));

		/* 请求事件(evt)的任务已完成,删掉它. */
		free_evt(evt);
		xmq_msg_destroy(msg);

		/* 发送增量响应结果给指定客户端. */
		assert(0 == send_evt(clinfo->evt_ctx, ev_rsp));

		return EVT_STATE_INCR;
	}
}

int do_dump_req_task(client_info_t *clinfo)
{
	evt_t   *evt = NULL;
	QITEM   *item = qlist_pull(&clinfo->qdump);

	assert(item);
	evt = item->data;
	qitem_free(item);
	assert(evt);
	assert(evt->ev_type == NET_EV_DUMP_REQ && evt->ev_state == NET_EV_NONE);

	/*发送所有dump*/
	assert(0 == send_evt(clinfo->evt_ctx, evt));
	clinfo->waits++;

	return EVT_STATE_DUMP;
}

int do_dump_rep_task(client_info_t *clinfo)
{
	evt_t *evt = NULL;

	while (true) {
		QITEM *item = qlist_pull(&clinfo->qdump);

		if (!item) {
			break;
		}

		evt = item->data;
		qitem_free(item);
		assert(evt);
		assert((evt->ev_type == NET_EV_DUMP_REP) ||
			(evt->ev_type == NET_EV_DUMP_REQ && evt->ev_state == NET_EV_NONE));

		/*发送所有dump*/
		if (evt->ev_type == NET_EV_DUMP_REQ) {
			assert(0 == send_evt(clinfo->evt_ctx, evt));
			clinfo->waits++;
		} else {
			/* 交换ID */
			x_printf(D, "====Before: evt->id=[%s] evt->ev_data=[%s].\n", evt->id, evt->ev_data);

			if (strlen(evt->ev_data) >= strlen(evt->id)) {
				char buf[IDENTITY_SIZE] = { 0 };
				strcpy(buf, evt->id);
				strcpy(evt->id, evt->ev_data);
				strcpy(evt->ev_data, buf);
			} else {
				int     ev_len = strlen(evt->id) + 1;
				evt_t   *ev_new = evt_new_by_size(ev_len);
				assert(ev_new);
				memcpy(ev_new, evt, evt_head_size());
				strcpy(ev_new->id, evt->ev_data);
				strcpy(ev_new->ev_data, evt->id);
				ev_new->ev_size = ev_len;

				free_evt(evt);
				evt = ev_new;
			}

			x_printf(D, "====After : evt->id=[%s] evt->ev_data=[%s].\n", evt->id, evt->ev_data);

			if (evt->ev_state != NET_EV_SUCC) {
				x_printf(W, "Last id:[%s] EV_DUMP_REQ has returned, but is fatal error.", evt->id);
				evt->ev_type = NET_EV_DUMP_REP;
				evt->ev_state = NET_EV_FAIL;
				x_printf(E, "SERVER->DO_DUMP_REQ: Execute fail. Try other destnation node?");
				assert(0 == send_evt(clinfo->evt_ctx, evt));
			} else {
				/* 设置时间的同步序列码 */
				evt->incr.task_seq = clinfo->consumer->last_fetch_seq;

				/* 修改数据库的值 */
#if 0
				client_info_t   *p_info = NULL;
				size_t          vlen = sizeof(p_info);

				bool ok = hashmap_get(clinfo->hmap, (void *)evt->id, strlen(evt->id), (void *)&p_info, &vlen);
				assert(ok && p_info);
				int res = xmq_update_that(p_info->consumer, EVENT_SEQ(evt));
#else
				xmq_consumer_t *p_consumer = xmq_get_consumer(clinfo->xmq_ctx, evt->id);

				if (!p_consumer) {
					x_printf(I, "This is the first time to create '%s' XMQ Consumer.", evt->id);

					assert( !xmq_register_consumer(clinfo->xmq_ctx, evt->id) );

					p_consumer = xmq_get_consumer(clinfo->xmq_ctx, evt->id);
					assert(p_consumer != NULL);
					x_printf(I, "Execute succeed and get consumer okay! ");
				}
				int res = xmq_update_that(p_consumer, EVENT_SEQ(evt));
#endif
				assert(res == 0);

				/* 客户端执行DUMP成功后,正常返回. */
				x_printf(I, "DO_DUMP_RSP: Send dump result to the request client.");
				print_evt(evt);
				assert(0 == send_evt(clinfo->evt_ctx, evt));
			}

			clinfo->waits--;
		}
	}

	/* 如果所有的DUMP请求都返回并发送给指定客户端了,我们需要回到同步状态. */
	return (clinfo->waits == 0) ? EVT_STATE_WAIT : EVT_STATE_DUMP;
}

static bool task_lookup(void *user, void *task)
{
	tlpool_t        *pool = user;
	bool            ok = false;
	int             idx = tlpool_get_thread_index(pool);

	ok = tlpool_pull(pool, task, TLPOOL_TASK_ALONE, idx);

	if (ok) {
		printf("get from alone queue!\n");
		return ok;
	}

	return ok;
}

/* 业务线程:
 *   具体的业务处理函数(内部可根据实际的业务情况,做协程方面的切换;
 *   所有的业务处理都在内部执行;
 *   含协程,处理客户端大量连接到服务器时的宏观上并发高效处理;
 *   1.处理普通的增量同步;
 *   2.处理客户端的DUMP请求和反馈;
 * */
void *task_handle(struct supex_evcoro *evcoro, int step)
{
	struct online_task      *p_task = &((struct online_task *)evcoro->task)[step];
	int                     idx = tlpool_get_thread_index(evcoro->data);
	client_info_t           *p_info = p_task->addr;

	printf("thread %lx index %d get [%p]\n", pthread_self(), idx, p_info);
	printf("------------华丽的分割线------------\n");

	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;

	int have = 0;

	while (true) {
		switch (p_info->state)
		{
			case EVT_STATE_INCR:
				/* 已经成功发送了增量响应,执行DUMP请求之前,需等待客户端结果返回; */
				/* 将所有DUMP请求,添加到qdump中,直到本次的增量请求返回时,才可以进入发送DUMP请求; */
				have = qlist_view(&p_info->qevts);

				if (have) {
					p_info->state = do_incr_rep_task(p_info);
				}

				break;

			case EVT_STATE_WAIT:
				have = qlist_view(&p_info->qevts);

				if (have) {
					/* 是否重启检查 */
					p_info->state = do_incr_chk_task(p_info);
				} else {
					/* 检验: 在新的增量请求之前,是否有DUMP操作待处理. */
					have = qlist_view(&p_info->qdump);

					if (have) {
						/* 客户端Node1 向服务器请求DUMP master节点, 但在event_dispenser.c中
						 * 通过调换(ID) 将Node1的事件 直接放入了master所在的线程事件队列中*/

						/* 执行DUMP请求前,需要等待上次的增量响应结果返回(也就是客户端返回成功)
						 * 另外,我们需要将客户端新的增量请求缓存起来,等待执行完DUMP请求之后再来处理; */
						p_info->state = do_dump_req_task(p_info);
					} else {
						/* 新的增量请求缓存起来,等DUMP请求处理完后,再做处理.*/
						have = qlist_view(&p_info->qincr);

						if (have) {
							p_info->state = do_incr_req_task(p_info);
							if (p_info->state == EVT_STATE_WAIT) {
								union evcoro_event event = {};
								evcoro_timer_init(&event, 0.005);
								evcoro_idleswitch(p_scheduler, &event, EVCORO_TIMER);
							}
						}
					}
				}

				break;

			case EVT_STATE_DUMP:
				have = qlist_view(&p_info->qevts);

				if (have) {
					/* 是否重启检查 */
					p_info->state = do_incr_chk_task(p_info);
				} else {
					have = qlist_view(&p_info->qdump);

					if (have) {
						/*
						 * 将DUMP请求队列中的所有事件都发送给对端.
						 * 同时,在接收客户端的DUMP执行结果时,也只需要一个即可!
						 * 同时刻发起的DUMP请求,客户端只需要执行一次,不需要执行多次DUMP.
						 */

						/* 从master客户端执行完DUMP操作后,master对应的任务处理会收到DUMP_REP,
						 * 服务器将ev_data拷贝给id,并且将结果PUSH到id对应的客户端. */
						p_info->state = do_dump_rep_task(p_info);
					}
				}

				break;

			case EVT_STATE_RUIN:
				xmq_unregister_consumer(p_info->xmq_ctx, p_info->id);

			case EVT_STATE_EXIT:
				/* hashmap.h 中并未提供删除HASH的操作,所以,我们只能将其值设置为NULL */
			{
				void *info = NULL;
				hashmap_set(p_info->hmap, (void *)p_info->id, strlen(p_info->id), (void *)&info, sizeof(info));

				client_info_destroy(p_info);
			}
				return NULL;

			default:
				return NULL;
		}

		if (have) {
			evcoro_fastswitch(p_scheduler);
		} else {
			union evcoro_event event = {};
			evcoro_timer_init(&event, 0.005);
			evcoro_idleswitch(p_scheduler, &event, EVCORO_TIMER);
		}
	}
}

void event_handler_startup(void *data)
{
	tlpool_t *pool = data;

	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct evcs_argv_settings sets = {
		.num    = g_pole_conf.event_tasker_counts	/*协程数*/
		, .tsz  = sizeof(struct online_task)
		, .data = data
		, .task_lookup= task_lookup
		, .task_handle= task_handle
	};
	EVCS_MODULE_CARRY(evcs, &sets);
	EVCS_MODULE_MOUNT(evcs);
	EVCS_MODULE_ENTRY(evcs, true);

	/*work events*/
	EVCS_MODULE_START();
}

// 客户端信息相关操作
// -- 创建一个未完全初始化的client_info_t,其他内部成员,需要手动赋值;
client_info_t *client_info_create(const char *id)
{
	client_info_t *clinfo = (client_info_t *)calloc(1, sizeof(client_info_t));

	if (clinfo != NULL) {
		strncpy(clinfo->id, id, sizeof(clinfo->id));
	}

	return clinfo;
}

void client_info_init(client_info_t *p_info)
{
	qlist_init(&p_info->qdump);
	qlist_init(&p_info->qevts);
	qlist_init(&p_info->qincr);
	p_info->state = EVT_STATE_INCR;
	p_info->waits = 0;
}

int client_info_destroy(client_info_t *clinfo)
{
	if (clinfo) {
		free(clinfo);
	}

	return 0;
}

