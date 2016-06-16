#include "event_handler.h"
#include "event_dispenser.h"
#include "slog/slog.h"

#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

enum destroy_ev_type
{
	DES_EV_SENDOK = 0,
	DES_EV_FAIL = -1,
	DES_EV_INCR_WAIT = 1
};

#define EVENT_SEQ(ev) ((ev)->incr.task_seq)

#define EXEC_FLOW_ERROR(ev)								  \
	do {										  \
		x_printf(F, "Execution flow error: Never should be here. Process exit."); \
		print_event(ev);							  \
		delete_event(ev); abort();						  \
	} while (0)

// 用于保存(KEY: 线程启动顺序0 ~ N-1 VALUE: 存储的每个线程对象 thread_member_t的指针
static hashmap_t *g_hash_thread = NULL;

// 线程
static void *running(void *args);

// 协程处理相关业务函数
static void idle(struct evcoro_scheduler *evcoro_sched, void *args);

static void task(struct evcoro_scheduler *evcoro_sched, void *args);

// 服务端主要干的三件事(其中do_incr_rsp,主要用于重发信息给客户端)
static int do_incr_req(thread_member_t *pthrd, client_info_t *info, event_t *ev);

static int do_incr_rsp(thread_member_t *pthrd, client_info_t *info, event_t *ev);

static int do_dump_req(thread_member_t *pthrd, client_info_t *info, event_t *ev);

static int do_dump_rsp(thread_member_t *pthrd, client_info_t *info, event_t *ev);

/* 辅助性函数 */
static void destroy_evcache(client_info_t *info, int type);

static int fetch_incr2send(thread_member_t *pthrd, client_info_t *info, event_t *ev);

static int send_error2client(event_ctx_t *ev_ctx, const char *id, int ev_type, int ev_state, char *ev_err);

/* 主线程调用,将启动(threads)个子线程(内含协程集群); */
int event_handler_startup(event_ctx_t *evctx, xmq_ctx_t *xctx, int threads)
{
	assert(evctx != NULL && xctx != NULL && threads > 0);

	// 初始化 HASH 结构,用于存储指定的线程标识(KEY)对应 thread_member_t 类型的指针(VALUE);
	g_hash_thread = hashmap_open();

	if (!g_hash_thread) {
		x_printf(E, "event_handler_startup: Create Thread Hash fail!");
		return -1;
	}

	int i, res = -1;

	/* 启动所有线程(内含协程处理) */
	for (i = 0; i < threads; i++) {
		/* 创建一个线程对象,并将其传入线程内部使用; */
		thread_member_t *pthrd = thread_member_create(g_hash_thread, i, threads, evctx, xctx);

		// 启动线程(内涵协程集群操作相关业务
		pthread_t thrd = 0;
		res = pthread_create(&thrd, NULL, running, (void *)pthrd);
		usleep(5);

		if (res != 0) {
			x_printf(F, "EVENT_HANDLER_STARTUP: Startup Thread fail! Error-%s.", strerror(errno));
			return -1;
		}

		x_printf(I, "EVENT_HANDLER_STARTUP: Create Thread<0x%lx> to handle evcoro successed.", thrd);
	}

	/* 加载已同步的所有客户端状态信息,如果有的话(用于服务器重启的情况) */
	xmq_consiter_t iter = xmq_get_consumer_iterator(xctx);

	while (iter.has_next(xctx)) {
		xmq_consumer_t *p_cons = iter.next(xctx);

		client_info_t *p_info = client_info_create(p_cons->identity);
		list_init(&p_info->list);

		p_info->tid = get_thread_id_by(threads, p_info->id);
		p_info->evp_dumps = evpipe_init();
		p_info->consumer = p_cons;
		p_info->sync_seq = p_cons->last_fetch_seq;

		thread_member_t *thrd = thread_member_getby(g_hash_thread, p_info->tid);

		/* 将客户端状态信息对象,添加到目标线程对象中.*/
		int res = client_info_add(thrd, p_info->id, p_info);
		assert(!res && p_info->evp_dumps && thrd);

		x_printf(I, "EVENT_HANDLER_STARTUP: Loading consumer<%s> to Thread<%d> succeed, last-seq:%ld.",
			p_info->id, p_info->tid, p_info->sync_seq);
	}

	return 0;
}

/* 业务线程:
 *   所有的业务处理都在内部执行;
 *   含协程,处理客户端大量连接到服务器时的宏观上并发高效处理;
 *   1.处理普通的增量同步;
 *   2.处理客户端的DUMP请求和反馈;
 * */
static void *running(void *args)
{
	thread_member_t *pthrd = (thread_member_t *)args;

	assert(pthrd != NULL);

	// 创建协程对象; -1: 代表默认缓存10个协程任务同时执行;
	pthrd->evcoro_sched = evcoro_create(-1);

	if (!pthrd->evcoro_sched) {
		x_printf(E, "running: Thread internal, create event coroutine scheduler fail. Thread exit!");
		return NULL;
	}

	while (true) {
		/* 迭代事件链表的所有事件(这些事件是由push_event_to_thread()设置,
		 * 并添加到协程集群中,内部调度处理;*/
		int ev_nodes = 0;

		xlist_t *iter = NULL;
		list_foreach(iter, &pthrd->list_events)
		{
			/* 主线程往队列中添加新事件,这里迭代读取,为了避免加锁,所以,
			 * 我们必须判断event_node_t的最后一个元素是否被设值;*/
			if (!iter->next || !iter->prev) {
				break;
			}

			++ev_nodes;

			list_del(iter);	/* 从链表中断开,而并不是删除数据;*/
			event_node_t *ev_node = container_of(iter, event_node_t, list);
			evcoro_push(pthrd->evcoro_sched, task, (void *)ev_node, 0);
		}

		/* 没事件产生时,休眠,轮询(客户端未连接之前,这里可能会休眠,如果没有新的增量数据,则不会休眠);*/
		if (ev_nodes < 1) {
			usleep(50000); continue;
		}

		DBG_PRINTS("Thread[%d]: There are [%d] coroutines to be processed.\n", pthrd->tid, evcoro_workings(pthrd->evcoro_sched));

		/* 启动协程队列,执行完后再次循环,增加新的事件请求,并再次协程批量处理;*/
		evcoro_loop(pthrd->evcoro_sched, NULL /*idle*/, NULL);
		usleep(5000);
	}

	evcoro_destroy(pthrd->evcoro_sched, NULL);
}

/* 具体的业务处理函数(内部可根据实际的业务情况,做协程方面的切换;*/
static void task(struct evcoro_scheduler *evcoro_sched, void *args)
{
	event_node_t *ev_node = (event_node_t *)args;

	assert(ev_node != NULL);

	thread_member_t *thrd = ev_node->thrd;
	event_t         *ev = ev_node->ev;

	/* 删除节点,但未删除其内部的[event_t/thread_member_t]指针;*/
	free(ev_node); ev_node = NULL;

	/* 通过事件ID查询其对应客户端(client_info_t)的对象; 每次肯定查找到.
	 * 因为即使执行DUMP请求,也会在PUSH到线程之前,调换id和ev_data的位置. */
	client_info_t *clinfo = client_info_get_by(thrd->hash_clinfo, ev->id);

	if (clinfo == NULL) {
		const char *err_info = "Client should send EV_DUMP_REQ first.";
		x_printf(W, "TASK: %s, I has already send error to it.", err_info);
		//		send_error2client(thrd, ev->id, NET_EV_INCREMENT_REP, NET_EV_FAIL, err_info);
		delete_event(ev);
		return;
	}

	/* do_xxx_xxx 返回-1代表客户端出现了严重错误,需要将该客户端对象
	 *   从其所在线程对象中删除. */
	switch (ev->ev_type)
	{
		case NET_EV_INCREMENT_REQ:

			/* 暂时只考虑增量同步时的严重错误(-1). DUMP_REP也可能出现. */
			if (do_incr_req(thrd, clinfo, ev) == -1) {
				client_info_delete_by(thrd, ev->id);
				x_printf(I, "TASK: Delete client<%s> state object and destroy it.", ev->id);
				goto TASK_ERROR;
			}

			break;

		/* 处理由于网络异常导致未成功发送出去的增量响应事件. */
		case NET_EV_INCREMENT_REP:

			if (do_incr_rsp(thrd, clinfo, ev)) {
				goto TASK_ERROR;
			}

			break;

		/* 客户端Node1 向服务器请求DUMP master节点, 但在event_dispenser.c中
		 * 通过调换(ID) 将Node1的事件 直接放入了master所在的线程事件队列中*/
		case NET_EV_DUMP_REQ:

			if (do_dump_req(thrd, clinfo, ev)) {
				goto TASK_ERROR;
			}

			break;

		/* 从master客户端执行完DUMP操作后,master对应的任务处理会收到DUMP_REP,
		 * 服务器将ev_data拷贝给id,并且将结果PUSH到id对应的线程,之后的事件还
		 * 会进入到当前处理位置,只不过是不同的线程,此时,就可以创建它的状态信息. */
		case NET_EV_DUMP_REP:

			if (do_dump_rsp(thrd, clinfo, ev)) {
				goto TASK_ERROR;
			}

			break;

		default:
			x_printf(W, "Invalid event type, It will be destroied.");
			delete_event(ev);
	}

	return;

TASK_ERROR:
	x_printf(F, "Processing the Client request fail. The request event will be deleted.");
	delete_event(ev);
}

static void idle(struct evcoro_scheduler *evcoro_sched, void *args)
{
	struct timeval tv = {};

	gettimeofday(&tv, NULL);

	DBG_PRINTS("\x1B[1;31m" "<<< %ld.%06ld : IDLE, >>>" "\x1B[m" "\n", tv.tv_sec, tv.tv_usec);

	if (evcoro_workings(evcoro_sched) < 1) {
		DBG_PRINTS("IDLE: No working evcoros, %d(s) was suspend, evcoro stopped.\n", evcoro_suspendings(evcoro_sched));
		evcoro_stop(evcoro_sched);
		sleep(1);	// TODO 每次处理完都会休眠1s.待改进!
	}
}

static int do_incr_req(thread_member_t *pthrd, client_info_t *info, event_t *ev)
{
	assert(pthrd != NULL && info != NULL && ev != NULL);

	/* 节点出现重大错误 */
	if (ev->ev_state == NET_EV_FATAL) {
		x_printf(F, "A fatal error[EV_FATAL] occured with client [%s]."
			" Will destroy the Client sync-state.", info->id);
		return -1;
	}

	/* 节点出现一般性错误 */
	if (ev->ev_state == NET_EV_FAIL) {
		x_printf(F, "A generality error[EV_FAIL] occured with client [%s]."
			" Cache it to ev_fail.", info->id);
		assert(info->ev_fail == NULL);
		info->ev_fail = ev;
		return 1;
	}

	uint64_t curr_seq = 0, last_seq = 0;

	/* 处理客户端重启时的各种情况 */
	if ((ev->ev_state == NET_EV_NONE) && (EVENT_SEQ(ev) == 0)) {
		/* 上次客户端出现了一般性错误 */
		if (info->ev_fail && (info->ev_fail->ev_state == NET_EV_FAIL)) {
			curr_seq = EVENT_SEQ(info->ev_fail);
			destroy_evcache(info, DES_EV_FAIL);
		}
		/* 上次正在执行DUMP请求,可能已经发送出去了,没收到DUMP_REP,或者还有未处理的DUMP请求.*/
		else if (info->ev_incr_wait) {
			// 不管怎样,客户端肯定是宕掉了,具体原因不详!
			// 长时间等待,请求DUMP的(其他)客户端,可自行断开和服务器的连接;
			curr_seq = EVENT_SEQ(info->ev_incr_wait);
			destroy_evcache(info, DES_EV_INCR_WAIT);
		}

		/* 上次客户端出现自己宕掉或被意外KILL的情况,那时,对应该客户端的服务器线程可能已经:
		 *  1.执行完增量同步发送 2.执行完DUMP请求 */
		else if (info->ev_send_okay) {
			switch (info->ev_send_okay->ev_type)
			{
				/* 场景一: 已完成增量同步的相应.
				 *     不管上次是否成功发送 NET_EV_INCREMENT_REP，都将重发该事件
				 *     模式: A(C) -> A(S) -> A(C) C代表客户端; S代表服务器 */
				case NET_EV_INCREMENT_REP:

					if (send_event(pthrd->ev_ctx, info->ev_send_okay)) {
						x_printf(E, "DO_INCR_REQ: send_event fail, when send the NET_EV_INCREMENT_REP again.");
						print_event(info->ev_send_okay);
						delete_event(ev);
						return 1;
					}

					delete_event(ev);
					return 0;

				/* 场景二: 已发送完DUMP请求.
				 *     服务器传递DUMP请求给(执行DUMP操作的客户端A时,A客户端意外被KILL,或自己宕掉.
				 *     如果上次发送的是NET_EV_DUMP_REQ，那么在前面( else if(info->ev_incr_wait)处
				 *     就被处理掉了!
				 *     模式: B(C) -> A(S) -> A(C) */
				case NET_EV_DUMP_REQ:
					DBG_PRINT("DO_INCR_REQ: Warning: Never should be here! It was handled before!\n");
					assert(false);
					break;

				/* 无效类型事件 */
				default:
					EXEC_FLOW_ERROR(ev);
			}
		}
		/* 第一次启动 */
		else {
			/* 节点在DUMP成功后首次启动, 或者master节点所对应的线程首次执行增量同步.*/
			assert(!info->ev_send_okay && !info->ev_incr_wait && !info->ev_fail);
			/* info->sync_seq 在创建客户端状态信息时,被设置 */
			curr_seq = info->sync_seq;
		}

		/* 设置时间的同步序列码 */
		ev->incr.task_seq = curr_seq;
	}
	/* 正常同步 */
	else if (info->ev_send_okay) {
		/* 正常同步时,只做两件事,1)INCR_REP 2)DUMP_REQ,而且它们都发送给了对端客户端;*/
		switch (info->ev_send_okay->ev_type)
		{
			case NET_EV_INCREMENT_REP:

				/* 检验: 在新的增量请求之前,是否有DUMP操作待处理. */
				if (evpipe_length(info->evp_dumps) > 0) {
					/* 先把新的增量请求缓存起来,等DUMP请求处理完后,再做处理.*/
					assert(info->ev_incr_wait == NULL);
					info->ev_incr_wait = ev;

					/* 将DUMP请求队列中的所有事件都发送给对端.此时,ev_send_okay只能保存最后一个,
					 * 同时,在接收客户端的DUMP执行结果时,也只需要一个即可!同时刻发起的DUMP请求
					 * 客户端只需要执行一次,不需要执行多次DUMP.*/
					event_t *ev_dump = NULL;

					while ((ev_dump = evpipe_pull(info->evp_dumps)) != NULL) {
						assert(ev_dump != NULL);

						if (send_event(pthrd->ev_ctx, ev_dump)) {
							x_printf(E, "Send Event Error: First dump-req send fail, after the incr-rsp!");
							print_event(ev_dump);
							x_printf(E, "We push the dump-req event to evp_dumps again.");
							evpipe_push_head(info->evp_dumps, ev_dump);
							return 1;
						}

						x_printf(I, "EV_DUMP_REQ for [%s] send to [%s] succeed.", ev_dump->ev_data, ev_dump->id);
						print_event(ev_dump);
						delete_event(info->ev_send_okay);
						info->ev_send_okay = ev_dump;
					}

					return 0;
				}

				/* 如果上一次发送的是NET_EV_INCREMENT_REP，那么ev_send_okay中获取上一次发送消息的seq */
				curr_seq = EVENT_SEQ(ev);
				// last_seq = EVENT_SEQ(info->ev_send_okay);

				/* 检查当前的请求序列是不是上次的值加1. */
				// assert(curr_seq == last_seq+1);

				/* 设置时间的同步序列码 */
				ev->incr.task_seq = curr_seq;
				break;

			/* 如果上一次发送的是NET_EV_DUMP_REQ，那么我们需要将当前的增量请求存储起来,
			 *   直到所有的DUMP执行完成为止; 因为增量同步是串行的,所以,只需要用一个变量保存*/
			case NET_EV_DUMP_REQ:
				x_printf(I, "If ev_send_okay is EV_DUMP_REQ, then we save the incr-event to ev_incr_wait.");
				info->ev_incr_wait = ev;
				return 0;

			default:
				EXEC_FLOW_ERROR(ev);
		}
	}
	/* 这种情况属于最开始启动时,队列中没有数据,或者执行完所有的DUMP响应后,我们什么都不需要做.*/
	else if (!info->ev_send_okay) {
		DBG_PRINT("DO_INCR_REQ: Here we need do nothing, go on....\n");
	}

	/*  客户端如果出错,会将错误状态保存在ev_cache中,但重新接收时,会在上面的 重启部分给处理掉
	 *    所以,该情况不可能发生!*/
	else {
		EXEC_FLOW_ERROR(ev);
	}

	/* 获取持久化队列中的消息，发送给对端节点. 成功返回0, 错误返回 -1 */
	return fetch_incr2send(pthrd, info, ev);
}

/* 处理由于网络原因导致的相关未发送成功的增量响应事件.*/
static int do_incr_rsp(thread_member_t *pthrd, client_info_t *info, event_t *ev)
{
	assert(pthrd != NULL && info != NULL && ev != NULL);
	assert(info->ev_incr_wait == NULL && info->ev_fail == NULL);

	if (send_event(pthrd->ev_ctx, ev)) {
		x_printf(F, "DO_INCR_RSP: Network serious anomaly, we'll try to send it later.");
		push_event_to_thread(ev, info->tid);
		return -1;
	}

	x_printf(I, "DO_INCR_RSP: We send the last INCR_REP succeed.");
	delete_event(info->ev_send_okay);
	info->ev_send_okay = ev;

	return 0;
}

/* 执行DUMP请求前,需要等待上次的增量响应结果返回(也就是客户端返回成功)
 * 另外,我们需要将客户端新的增量请求缓存起来,等待执行完DUMP请求之后再来处理; */
static int do_dump_req(thread_member_t *pthrd, client_info_t *info, event_t *ev)
{
	assert(pthrd != NULL && info != NULL && ev != NULL);
	assert(ev->ev_type == NET_EV_DUMP_REQ && ev->ev_state == NET_EV_NONE);

	/* 上次获取到了增量返回事件,且状态为EV_FAIL. */
	if (info->ev_fail) {
		x_printf(F, "Client Exeception: We cann't send DUMP_REQ now!");
		send_error2client(pthrd->ev_ctx, ev->ev_data, NET_EV_DUMP_REP, NET_EV_FAIL,
			"SERVER->DO_DUMP_REQ: Execute fail. Destination-Client incr-rep return fail.");
		// 这里不删除info->ev_fail的值,因为,此时,可能还有其他的客户端向该客户端请求DUMP操作;
		// 等该客户端重启后,info->ev_fail,会在do_incr_req中被删除;
		delete_event(ev);
		return 0;
	}
	/* 上次可能: 1.已发送了一个增量响应(不知是否已返回); 2.发送了一个DUMP请求. */
	else if (info->ev_send_okay) {
		switch (info->ev_send_okay->ev_type)
		{
			case NET_EV_INCREMENT_REP:
				/* 上次成功发送了增量响应,执行DUMP请求之前,需等待客户端结果返回; */
				/* 将该DUMP请求,添加到evp_dumps中,直到上次的增量请求返回时,才可以发送DUMP请求; */
				x_printf(I, "We send a INCR_REP last time, we need it return, "
					"so we put the dump event to evp_dumps at first.");

				evpipe_push_tail(info->evp_dumps, ev);
				DBG_PRINT("EV_DUMP_REQ: push event down, to evp_dumps.\n");
				print_event(ev);
				return 0;

			case NET_EV_DUMP_REQ:
				// 上次成功发送了DUMP请求,此时,再次请求DUMP,我们可以忽略它,直接返回成功的状态;
				x_printf(I, "The EV_DUMP_REQ has already been executed, don't need dump again.");

				/* 上次执行完DUMP请求后,如果没有DUMP请求,就将上次缓存的增量请求发送出去.*/
				if (evpipe_length(info->evp_dumps) == 0) {
					if (send_event(pthrd->ev_ctx, info->ev_incr_wait)) {
						x_printf(E, "Send event error: Send the cached INCR_REQ event fail.");
						print_event(info->ev_incr_wait);
						return -1;
					}

					destroy_evcache(info, DES_EV_SENDOK);
					info->ev_send_okay = info->ev_incr_wait;
					destroy_evcache(info, DES_EV_INCR_WAIT);
					return 0;
				}

			default:
				EXEC_FLOW_ERROR(info->ev_send_okay);
		}
	}
	/* 情况: 服务端(程序)已启动,此时XMQ队列中没有数据,此时,已接收到来自客户端的DUMP请求 */
	else if (!info->ev_send_okay) {
		if (send_event(pthrd->ev_ctx, ev)) {
			x_printf(E, "DO_DUMP_REQ: Send the DUMP_REQ event fail. when server-XMQ has no incr-data.");
			print_event(ev);
			return -1;
		}

		info->ev_send_okay = ev;
		return 0;
	} else {
		EXEC_FLOW_ERROR(ev);
	}

	return 0;
}

static int do_dump_rsp(thread_member_t *pthrd, client_info_t *info, event_t *ev)
{
	assert(pthrd != NULL && info != NULL && ev != NULL);

	if (ev->ev_state != NET_EV_SUCC) {
		x_printf(W, "Last id:[%s] EV_DUMP_REQ has returned, but is fatal error.", ev->ev_data);
		send_error2client(pthrd->ev_ctx, ev->ev_data, NET_EV_DUMP_REP, NET_EV_FAIL,
			"SERVER->DO_DUMP_REQ: Execute fail. Try other destnation node?");
		delete_event(ev);
		return (ev->ev_state == NET_EV_FATAL) ? -1 : 0;
	}

	/* 客户端执行DUMP成功后,正常返回. */
	assert(ev->ev_state != NET_EV_NONE);

	/* 通过客户端标识,创建发起DUMP请求的客户端对象.*/
	client_info_t *src_info = client_info_create(ev->ev_data);
	assert(src_info != NULL);

	/* 通过客户端标识,创建一个消费者对象. 因为,所有的线程公用同一个XMQ,所以,这里参数直接写了pthrd.*/
	if (xmq_register_consumer(pthrd->xmq_ctx, ev->ev_data)) {
		x_printf(E, "xmq_register_consumer: Execute fail. Client Id <%s>.", ev->ev_data);
		return -1;
	}

	src_info->consumer = xmq_get_consumer(pthrd->xmq_ctx, ev->ev_data);
	assert(src_info->consumer != NULL);
	x_printf(I, "DO_DUMP_REP: xmq_register_consumer: Execute succeed and get consumer okay!");

	src_info->evp_dumps = evpipe_init();
	assert(src_info->evp_dumps != NULL);
	x_printf(I, "DO_DUMP_REP: evpipe_init(): Create a evp_dumps succeed.");

	/* 通过客户端标识及求余算法,获取对应的线程ID. */
	src_info->tid = get_thread_id_by(pthrd->threads, ev->ev_data);

	/* 通过线程ID,获取对应的线程对象指针 */
	thread_member_t *src_thrd = thread_member_getby(g_hash_thread, src_info->tid);

	/* 将新创建的客户端状态信息对象,添加到目标线程对象中.*/
	int res = client_info_add(src_thrd, ev->ev_data, src_info);
	assert(res == 0);

	/* 更新ev->id为ev->ev_data,然后发送给客户端 */
	strcpy(ev->id, ev->ev_data);
	x_printf(I, "DO_DUMP_RSP: Send dump result to the request client.");
	print_event(ev);

	if (send_event(pthrd->ev_ctx, ev)) {
		x_printf(E, "DO_DUMP_RSP: Send dump result to client[%s] fail.", ev->id);
		delete_event(ev);
		return -1;
	}

	delete_event(ev);

	/* 如果所有的DUMP请求都返回并发送给指定客户端了,我们需要处理之前保存的ev_incr_wait. */
	if (evpipe_length(info->evp_dumps) == 0) {
		x_printf(I, "DO_DUMP_RSP: There's nomore EV_DUMP_REP to send, we will handle the info->ev_incr_wait.");

		/* 发送完所有的DUMP响应后,我们需要将ev_send_okay 销毁并清空,这样在do_incr_req中,
		 * 程序流会进入到 else if(!info->ev_send_okay)中,什么都不需要做,继续执行fetch_incr2send(). */
		destroy_evcache(info, DES_EV_SENDOK);

		/* 从XMQ队列中获取数据并发送. */
		assert(info->ev_incr_wait != NULL);
		res = fetch_incr2send(pthrd, info, info->ev_incr_wait);
		/* 用完后一定记得销毁或置空(fetch_incr2send()内部会决定ev_incr_wait的去留) */
		info->ev_incr_wait = NULL;
		return res;
	}

	return 0;
}

static int fetch_incr2send(thread_member_t *pthrd, client_info_t *info, event_t *ev)
{
	assert(pthrd != NULL && info != NULL && ev != NULL);

	int             fetch_times = 0;
	uint64_t        task_seq = EVENT_SEQ(ev);
	xmq_msg_t       *msg = NULL;
FETCH_AGAIN:
	msg = xmq_fetch_nth(info->consumer, task_seq, 0);

	if (!msg) {
		++fetch_times;
		//		DBG_PRINTS("Tid:<%d> XMQ has no new data to sync. Call evcoro_fastswitch The %dth times!\n", info->tid, fetch_times);
		evcoro_fastswitch(pthrd->evcoro_sched);	/* Let other coroutine do first.*/

		/* 要么轮训一次再次查看,如果没有我就得 push_event_to_thread().*/
		if (fetch_times < 3) {
			goto FETCH_AGAIN;
		}

		/* 将未处理的事件,重新放回指定线程队列里,待下次处理. */
		push_event_to_thread(ev, info->tid);
	} else {
		assert(xmq_msg_size(msg) > 0);

		/* 创建一个待发送的增量响应事件. */
		event_t *ev_rsp = event_new_size(xmq_msg_size(msg));

		if (ev_rsp == NULL) {
			x_printf(E, "event_new_size: Create new event fail.");
			return 1;
		}

		memcpy(ev_rsp, ev, event_head_size());
		ev_rsp->ev_size = xmq_msg_size(msg);
		ev_rsp->ev_type = NET_EV_INCREMENT_REP;
		memcpy(ev_rsp->ev_data, xmq_msg_data(msg), xmq_msg_size(msg));

		/* 请求事件(ev)的任务已完成,删掉它. */
		delete_event(ev);

		/* 发送增量响应结果给指定客户端. */
		if (send_event(pthrd->ev_ctx, ev_rsp)) {
			DBG_PRINT("FETCH_INCR2SEND: Sending INCR_REP fail. Elements like:\n");
			print_event(ev_rsp);
			x_printf(I, "FETCH_INCR2SEND: Network anomaly, we'll try to send it later! "
				"ID:<%s> INCR:<%s>", ev_rsp->id, ev_rsp->ev_data);
			push_event_to_thread(ev_rsp, info->tid);
			//			xmq_msg_destroy(xmq_fetch_nth(info->consumer, task_seq-1, 0));
			return 1;
		}

		/* 更新当前客户端的相关状态值. */
		delete_event(info->ev_send_okay);
		info->ev_send_okay = ev_rsp;	/* Update the ev_send_okay every time. */
		info->sync_seq = task_seq;	/* For client restart situation. */
	}

	return 0;
}

int push_event_to_thread(event_t *ev, int thread)
{
	if (!ev || (thread < 0)) {
		x_printf(E, "New Event:%p, thread:%d. was invalid.", ev, thread);
		return -1;
	}

	thread_member_t *pthrd = thread_member_getby(g_hash_thread, thread);
	assert(pthrd != NULL);
	DBG_PRINTS("PUSH_EVENT_TO_THREAD: Thread<%d>....\n", thread);
	print_event(ev);

	static
	bool master_okay = false;

	if (!master_okay) {
		int i = 0, clis = 0;

		for (; i < pthrd->threads; ++i) {
			clis += client_info_length(thread_member_getby(g_hash_thread, i));
		}

		/* 如果程序首次加载时,已经有客户端存在,那么说明,服务器已重启. */
		if (clis > 0) {
			x_printf(I, "Pole-M has already restart with %d clients loaded.", clis);
		}
		/* 否则是第一次启动*/
		else if (!strcmp(ev->id, "master")) {
			/* 执行当前函数之前,几个线程肯定已经启动,并将0~threads的线程号和线程对象地址保存在了HASH表中. */
			thread_member_t *p_thrd = thread_member_getby(g_hash_thread, thread);
			assert(p_thrd != NULL);

			client_info_t *p_info = client_info_get_by(p_thrd->hash_clinfo, ev->id);

			if (p_info == NULL) {
				/* Create master's client infor state.*/
				p_info = client_info_create(ev->id);
				assert(p_info != NULL);

				p_info->tid = thread;
				p_info->evp_dumps = evpipe_init();
				assert(p_info->evp_dumps != NULL);

				p_info->consumer = xmq_get_consumer(pthrd->xmq_ctx, ev->id);

				if (!p_info->consumer) {
					x_printf(I, "xmq_get_consumer: fail! This is the first time to create 'master' XMQ Consumer.");

					if (xmq_register_consumer(pthrd->xmq_ctx, ev->id)) {
						x_printf(E, "xmq_register_consumer: Execute fail. Client Id <%s>.", ev->id);
						return -1;
					}

					p_info->consumer = xmq_get_consumer(pthrd->xmq_ctx, ev->id);
					assert(p_info->consumer != NULL);
					x_printf(I, "xmq_register_consumer: Execute succeed and get consumer okay! ");
				}

				client_info_add(pthrd, ev->id, p_info);
			}
		}
		/* 第一次连上来的一定是主节点. */
		else {
			x_printf(W, "First request client must be 'master'. " \
				"Checking the pole-S_conf.json file's SLAVE_UNIQUE_ID");
			return -1;
		}

		/* 下次就不用在进来判断了. */
		master_okay = true;
	}

	/* 添加新的事件到事件链表,供协程处理; */
	event_node_t *ev_node = (event_node_t *)calloc(1, sizeof(event_node_t));
	assert(ev_node != NULL);

	ev_node->ev = ev;
	ev_node->thrd = pthrd;

	list_add_tail(&ev_node->list, &pthrd->list_events);

	return 0;
}

// TODO
int event_handler_destroy()
{
	hashmap_close(g_hash_thread);
	g_hash_thread = NULL;

	return 0;
}

// 线程对象相关操作
thread_member_t *thread_member_create(hashmap_t *h_thrd, int id, int threads, event_ctx_t *evctx, xmq_ctx_t *xctx)
{
	assert(h_thrd != NULL && id > -1 && threads > 0 && evctx != NULL && xctx != NULL);

	thread_member_t *pthrd = calloc(1, sizeof(*pthrd));
	assert(pthrd != NULL);

	// 添加线程ID,线程对象到HASH表, VALUE保存的是地址;
	hashmap_set(h_thrd, (void *)&id, sizeof(int), (void *)&pthrd, sizeof(pthrd));

	pthrd->tid = id;
	pthrd->threads = threads;

	list_init(&pthrd->list);		// TODO:
	list_init(&pthrd->list_events);		// 初始化事件队列
	list_init(&pthrd->list_clinfo);		// 初始化客户端状态队列

	pthrd->ev_ctx = evctx;			// 网络事件上下文,用于收发事件;
	pthrd->xmq_ctx = xctx;			// XMQ上下文句柄,用于创建XMQ队列的消费者;

	// HASH存储本线程的所有客户端状态信息.
	pthrd->hash_clinfo = hashmap_open();
	assert(pthrd->hash_clinfo != NULL);

	// 协程的创建和使用必须在同一个线程中(线程启动后,将在内部创建协程,并使用);
	pthrd->evcoro_sched = NULL;

	return pthrd;
}

int thread_member_destroy(thread_member_t *thrd)
{
	// TODO:
	return 0;
}

thread_member_t *thread_member_getby(hashmap_t *hash_thrd, int id)
{
	/* hashmap_get()内部对vlen的值做了筛选,所以这里必须设置一个值; */
	size_t          vlen = sizeof(void *);
	thread_member_t *pthrd = NULL;

	bool okay = hashmap_get(hash_thrd, (void *)&id, sizeof(int), (void *)&pthrd, &vlen);

	assert(okay == true);
	assert(pthrd != NULL && vlen == sizeof(void *));

	return pthrd;
}

////int thread_member_push_event(thread_member_t *thrd, event_t *ev)
// {
// }

// 客户端信息相关操作
// -- 创建一个未完全初始化的client_info_t,其他内部成员,需要手动赋值;
client_info_t *client_info_create(const char *id)
{
	client_info_t *clinfo = (client_info_t *)calloc(1, sizeof(client_info_t));

	if (clinfo != NULL) {
		strncpy(clinfo->id, id, sizeof(clinfo->id));
		clinfo->sync_seq = 1;
	}

	return clinfo;
}

int client_info_destroy(client_info_t *clinfo)
{
	int res = -1;

	if (clinfo != NULL) {
		// TODO:
	}

	return res;
}

// -- 新增一个client_info_t类型的HASH节点,并添加到 list_clinfo 中;
int client_info_add(thread_member_t *thrd, const char *id, client_info_t *clinfo)
{
	if (thrd && id && (strlen(id) > 0) && clinfo) {
		hashmap_set(thrd->hash_clinfo, (void *)id, strlen(id), (void *)&clinfo, sizeof(clinfo));
		list_add_tail(&clinfo->list, &thrd->list_clinfo);
		++thrd->size_clinfo;
	} else {
		x_printf(E, "client_info_add: Input parameter invalid.");
		return -1;
	}

	return 0;
}

// -- 删除一个客户端状态,并将其对应的HASH值 置空.
int client_info_delete_by(thread_member_t *thrd, const char *id)
{
	if (thrd && id && (strlen(id) > 0)) {
		client_info_t *clinfo = client_info_get_by(thrd->hash_clinfo, id);

		if (clinfo == NULL) {
			x_printf(W, "Client [%s] doesn't exist in client hashmap.", id);
			return -1;
		}

		list_del(&clinfo->list);	// 从list_clinfo中删除;
		client_info_destroy(clinfo);	// 销毁它;

		// hashmap.h 中并未提供删除HASH的操作,所以,我们只能将其值设置为NULL;
		clinfo = NULL;
		hashmap_set(thrd->hash_clinfo, (void *)id, strlen(id), (void *)&clinfo, sizeof(clinfo));
		--thrd->size_clinfo;
	} else {
		x_printf(E, "client_info_delete_by: Input parameter invalid.");
		return -1;
	}

	return 0;
}

// -- 通过一个客户端ID,获取对应的 client_info_t 的内存数据地址,这样就可以快速更新该客户端状态;
client_info_t *client_info_get_by(hashmap_t *hash_clinfo, const char *id)
{
	client_info_t *clinfo = NULL;

	if (hash_clinfo && id && (strlen(id) > 0)) {
		size_t vlen = sizeof(*clinfo);
		hashmap_get(hash_clinfo, (void *)id, strlen(id), (void *)&clinfo, &vlen);
	} else {
		x_printf(E, "client_info_get_by: Input parameter invalid.");
	}

	return clinfo;
}

int client_info_length(thread_member_t *thrd)
{
	return thrd->size_clinfo;
}

static void destroy_evcache(client_info_t *info, int type)
{
	assert(info != NULL && type >= -1 && type <= 1);

	switch (type)
	{
		case DES_EV_FAIL:
		{ delete_event((info)->ev_fail); (info)->ev_fail = NULL; break; }

		case DES_EV_SENDOK:
		{ delete_event((info)->ev_send_okay); (info)->ev_send_okay = NULL; break; }

		case DES_EV_INCR_WAIT:
		{ delete_event((info)->ev_incr_wait); (info)->ev_incr_wait = NULL; break; }

		default:
			break;
	}
}

static int send_error2client(event_ctx_t *ev_ctx, const char *id, int ev_type, int ev_state, char *ev_err)
{
	assert(ev_ctx && id && strlen(id) > 0);

	event_t *ev = event_new_size((ev_err) ? (strlen(ev_err) + 1) : 0);

	strcpy(ev->id, id);
	ev->ev_type = ev_type;
	ev->ev_state = ev_state;

	if (ev_err != NULL) {
		strcpy(ev->ev_data, ev_err);
	}

	if (send_event(ev_ctx, ev)) {
		x_printf(E, "SEND_ERROR2CLIENT: Send fail. id:<%s> err_info:<%s>.", ev->id, ((ev_err) ? ev->ev_data : ""));
		delete_event(ev);
		return -1;
	}

	return 0;
}

