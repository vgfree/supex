#include <stdlib.h>
#include <string.h>

#include "porter.h"
#include "slog/slog.h"

#include "xmq.h"
#include "netmod.h"

xlist_t         porters_list_head;
pthread_mutex_t porters_list_lock;

static event_t *make_mesg_response(event_t *mesgreq, xmq_msg_t *msg);

static event_t *make_dump_request(event_t *dumpreq);

static event_t *make_dump_response(event_t *dumprsp, char *dest);

static void porter_error_term(porter_info_t *info);

static void *do_work(void *args);

static int fetch_msg_to_send(porter_info_t *info, event_t *mesgreq);

static int do_mesg_request(porter_info_t *info, event_t *mesgreq);

static int do_dump_request(porter_info_t *info, event_t *dumpreq);

static int do_dump_response(porter_info_t *info, event_t *dumprsp);

extern void porters_list_init(void)
{
	list_init(&porters_list_head);
	pthread_mutex_init(&porters_list_lock, NULL);
}

extern void porters_list_destroy(void)
{
	pthread_mutex_lock(&porters_list_lock);

	while (!list_empty(&porters_list_head)) {
		xlist_t         *pick = porters_list_head.next;
		porter_info_t   *porter = container_of(pick, porter_info_t, list);

		list_del(pick);
		porter_destroy(porter);
	}

	pthread_mutex_unlock(&porters_list_lock);
	pthread_mutex_destroy(&porters_list_lock);
}

extern porter_info_t *look_up_porter(const char *peername)
{
	xlist_t         *node = NULL;
	porter_info_t   *porter = NULL;

	pthread_mutex_lock(&porters_list_lock);

	list_foreach(node, &porters_list_head)
	{
		porter = container_of(node, porter_info_t, list);

		if (!strncmp(porter->peer, peername, strlen(peername) + 1)) {
			break;
		} else {
			porter = NULL;
		}
	}

	pthread_mutex_unlock(&porters_list_lock);

	return porter;
}

extern porter_info_t *porter_create(const char *peer, xmq_consumer_t *consumer, event_ctx_t *transceiver, uint64_t start_seq)
{
	porter_info_t *porter = NULL;

	if (strlen(peer) && (strlen(peer) < sizeof(porter->peer))) {
		porter = (porter_info_t *)calloc(1, sizeof(porter_info_t));

		if (!porter) {
			return porter;
		}

		strncpy(porter->peer, peer, sizeof(porter->peer));

		porter->consumer = consumer;
		porter->transceiver = transceiver;
		porter->start_seq = start_seq;

		pipe_init(&porter->events);
		pipe_init(&porter->todump);
	}

	return porter;
}

extern void porter_destroy(porter_info_t *porter)
{
	if (porter) {
		if (porter->sending) {
			delete_event(porter->sending);
		}

		if (porter->reserved) {
			delete_event(porter->reserved);
		}

		pipe_release(&porter->events);
		pipe_release(&porter->todump);

		free(porter);
	}
}

static event_t *make_mesg_response(event_t *mesgreq, xmq_msg_t *msg)
{
	event_t *response = NULL;

	if (NET_EV_INCREMENT_REQ != mesgreq->ev_type) {
		x_printf(W, "event type is not NET_EV_INCREMENT_REQ");
		return response;
	}

	if (!xmq_msg_size(msg)) {
		x_printf(W, "msg size is 0");
		return response;
	}

	response = event_new_size(xmq_msg_size(msg));

	if (response) {
		memcpy(response, mesgreq, event_head_size());
		response->ev_size = xmq_msg_size(msg);
		response->ev_type = NET_EV_INCREMENT_REP;
		memcpy(response->ev_data, xmq_msg_data(msg), xmq_msg_size(msg));
	}

	return response;
}

static event_t *make_dump_request(event_t *dumpreq)
{
	event_t *request = NULL;

	if (NET_EV_DUMP_REQ != dumpreq->ev_type) {
		x_printf(W, "event type is not NET_EV_DUMP_REQ");
		return request;
	}

	if (dumpreq->ev_size >= sizeof(dumpreq->id)) {
		x_printf(W, "too long destination id [%s] to dump", dumpreq->ev_data);
		return request;
	}

	request = event_new_size(strlen(dumpreq->id) + 1);

	if (request) {
		strncpy(request->id, dumpreq->ev_data, sizeof(request->id));
		strncpy(request->ev_data, dumpreq->id, request->ev_size);
		request->ev_type = NET_EV_DUMP_REQ;
		request->ev_state = NET_EV_NONE;
	}

	return request;
}

static event_t *make_dump_response(event_t *dumprsp, char *dest)
{
	event_t *response = NULL;

	if (NET_EV_DUMP_REP != dumprsp->ev_type) {
		x_printf(W, "event type is not NET_EV_DUMP_REP");
		return response;
	}

	if ((strlen(dest) == 0) || (strlen(dest) >= sizeof(dumprsp->id))) {
		x_printf(W, "invalid destination id [%s] for it's len = %lu", dest, strlen(dest));
		return response;
	}

	response = event_new_size(event_body_size(dumprsp));

	if (response) {
		memcpy(response, dumprsp, event_total_size(dumprsp));
		strncpy(response->id, dest, sizeof(response->id));
	}

	return response;
}

#define event_seq(ev) ((ev)->incr.task_seq)

static void *do_work(void *data)
{
	event_t         *event = NULL;
	porter_info_t   *info = (porter_info_t *)data;

	if (info->reserved) {
		x_printf(W, "porter->reserved is not empty");
		delete_event(info->reserved);
	}

	if (info->sending) {
		x_printf(W, "porter->sending is not empty");
		delete_event(info->sending);
	}

	while (1) {
		event = pipe_pull(&info->events, 0);

		if (!event) {
			x_printf(E, "pipe_pull failed, error: [%s]", strerror(errno));
			goto PORTER_ERR;
		}

		x_printf(D, "work porter:");	// test
		print_event(event);		// test

		switch (event->ev_type)
		{
			case NET_EV_INCREMENT_REQ:

				if (do_mesg_request(info, event)) {
					goto PORTER_ERR;
				}

				break;

			case NET_EV_DUMP_REQ:

				if (do_dump_request(info, event)) {
					goto PORTER_ERR;
				}

				break;

			case NET_EV_DUMP_REP:

				if (do_dump_response(info, event)) {
					goto PORTER_ERR;
				}

				break;

			default:
				x_printf(W, "invalid event type [%d] at first", event->ev_type);
				delete_event(event);
		}
	}	// end while.

PORTER_ERR:
	porter_error_term(info);
	return NULL;
}

/*fetch_msg_to_send - 从持久化队列中获取数据，并发送给节点
 * @info:            工作线程信息
 * @mesgreq:          消息请求事件
 * return:          取得消息并发送成功返回1，错误返回-1，无数据超时返回0
 * */
static int fetch_msg_to_send(porter_info_t *info, event_t *mesgreq)
{
	xmq_msg_t       *msg = NULL;
	uint64_t        curr_seq = event_seq(mesgreq);

	while (1) {
		/*阻塞方式从持久化队列中取数据，[持久化队列是增量数据] 超时时间1000ms*/
		msg = xmq_fetch_nth(info->consumer, curr_seq, 1000);

		if (msg != NULL) {
			break;
		}

		if (msg == NULL) {
			event_t *next = NULL;

			/* 阻塞方式从pipe中取下一个事件，[此时，若有请求，肯定是NET_EV_DUMP类型] 超时时间1000ms*/
			next = pipe_pull(&info->events, 1000);

			if (next) {					// 取得事件
				x_printf(I, "We got a event request, when XMQ queue has no data to send, it must be NET_EV_DUMP type.");
				print_event(next);

				if (NET_EV_DUMP_REQ == next->ev_type) {	// 如果有下一个事件，那么必定是DUMP请求事件
					x_printf(I, "swap order with NET_EV_INCREMENT_REQ event and NET_EV_DUMP_REP event");

					/* 将当前的数据请求事件和DUMP请求事件交换顺序，塞回到pipe中
					 * 在do_work函数中的下次循环，将先取得DUMP事件执行处理，然后再
					 * 在下一个循环处理此数据请求事件
					 */
					pipe_push_back(&info->events, mesgreq);
					pipe_push_back(&info->events, next);

					return 0;
				} else {
					x_printf(W, "event with type [%d] must not follow the event of NET_EV_INCREMENT type", next->ev_type);
					delete_event(next);
					continue;
				}
			} else {// pipe中暂无其他事件，那么继续回到从持久化队列中取数据的地方获取数据
				continue;
			}
		} else {
			x_printf(E, "xmq_fetch_nth error!!!");
			delete_event(mesgreq);
			return -1;
		}
	}

	event_t *response = make_mesg_response(mesgreq, msg);
	xmq_msg_destroy(msg);

	if (response) {
		int res = send_event(info->transceiver, response);

		if (res) {
			x_printf(E, "send_event error [%s]", event_error(res));
			delete_event(response);

			/* 将持节化队列中数据序号回退 */
			xmq_msg_destroy(xmq_fetch_nth(info->consumer, curr_seq - 1, 0));
			return -1;
		}
	} else {
		x_printf(E, "make_mesg_response failed");

		/* 将持节化队列中数据序号回退 */
		xmq_msg_destroy(xmq_fetch_nth(info->consumer, curr_seq - 1, 0));
		return -1;
	}

	delete_event(info->sending);
	info->sending = response;

	return 1;
}

/*do_mesg_request - 数据请求处理函数
 * @info:          工作线程信息
 * @mesgreq:        数据请求事件
 * return:        成功返回0，失败返回-1，返回-1线程将推出并清除状态信息
 * */
static int do_mesg_request(porter_info_t *info, event_t *mesgreq)
{
	/* 节点出现重大错误 */
	if (NET_EV_FATAL == mesgreq->ev_state) {
		x_printf(F, "A fatal error occured with porter [%s].", info->peer);
		delete_event(mesgreq);
		return -1;	// 返回-1，让线程清理信息并退出
	}

	/* 节点出现一般性错误 */
	if (NET_EV_FAIL == mesgreq->ev_state) {
		x_printf(W, "A error occured with porter [%s].", info->peer);

		if (info->reserved) {
			x_printf(W, "info->reserved is not NULL");
			delete_event(info->reserved);
		}

		info->reserved = mesgreq;	// 保留当前事件，以便节点重启后继续同步
		return 0;			// 返回0，让线程继续保留信息并等待
	}

	uint64_t curr_seq = 0, last_seq = 0;

	/* 客户端发送NET_EV_NONE状态类型，包含3种情况：
	 *   1. 客户端执行完NET_EV_DUMP请求，并成功将数据文件恢复到数据库，修改配置想项，并再次启动增量NET_EV_INCREMENT 请求时;
	 *   2. 客户端业务执行失败，并退出，由运维人员解决问题后，再次启动客户端时;
	 *   3. 客户端进程被杀[SIG_KILL]或[Ctrl+C]，这个属意外情况，再次启动时;
	 */
	if ((NET_EV_NONE == mesgreq->ev_state) && (0 == event_seq(mesgreq))) {
		// 上次出现了一般性错误
		if (info->reserved && (NET_EV_FAIL == info->reserved->ev_state)) {
			curr_seq = (event_seq(info->reserved)) ? event_seq(info->reserved) : info->start_seq;
			delete_event(info->reserved);
		}
		// 节点停止运行后再次启动，并期望接着上次同步的地方继续同步
		else if (info->sending) {
			int res = -1;
			switch (info->sending->ev_type)
			{
				case NET_EV_INCREMENT_REP:	/* 如果上次发送的是 NET_EV_INCREMENT_REP，那么将上次发送的事件重发 */
					res = send_event(info->transceiver, info->sending);

					if (res) {
						x_printf(E, "send_event error [%s]", event_error(res));
						delete_event(mesgreq);

						/* 将持节化队列中数据序号回退 */
						xmq_msg_destroy(xmq_fetch_nth(info->consumer, event_seq(info->sending) - 1, 0));
						return -1;
					}

					delete_event(mesgreq);
					return 0;

				case NET_EV_DUMP_REQ:	/* 如果上次发送的是NET_EV_DUMP_REQ，那么从reserved里面获取当前seq */

					if (info->reserved) {
						curr_seq = event_seq(info->reserved);
						delete_event(info->reserved);
					} else {
						x_printf(W, "reserved is NULL after send NET_EV_DUMP_REQ event");
						curr_seq = xmq_fetch_position(info->consumer);
					}

					break;

				case NET_EV_DUMP_REP:	/* 如果上次发送的是NET_EV_DUMP_REP，那么只能从持久化队列中获取当前seq */
					curr_seq = xmq_fetch_position(info->consumer);
					curr_seq = (curr_seq < info->start_seq) ? info->start_seq : curr_seq;

					if (info->reserved) {
						x_printf(W, "reserved is not NULL after send NET_EV_DUMP_REP event");
						delete_event(info->reserved);
					}

					break;

				default:// 无效类型事件
					x_printf(E, "invalid event [%d] type for sending", info->sending->ev_type);
					delete_event(mesgreq);
					return -1;
			}
		} else {// 节点在DUMP成功后，首次启动, 或者master节点所对应的线程执行 NET_EV_INCREMENT 同步.
			curr_seq = info->start_seq;
		}
	} else if (info->reserved) {	/* 此种场景不可能发生，与客户端节点对应的线程在接受了一个NET_EV_FAIL事件之后,
					 *   客户端进程会自动退出，不会连续发送其他的请求. */
		if (NET_EV_FAIL == info->reserved->ev_state) {
			x_printf(W, "Can not continue SYNC data! There must be a sync error, wait for repair up!");
			delete_event(mesgreq);
			return 0;
		}

		x_printf(D, "the state [%d] of reserved event is not NET_EV_FAIL", info->reserved->ev_state);
		delete_event(mesgreq);
		return -1;
	}
	/* 正常同步 */
	else if (info->sending) {
		switch (info->sending->ev_type)
		{
			case NET_EV_INCREMENT_REP:	/* 如果上一次发送的是NET_EV_INCREMENT_REP，那么sending中获取上一次发送消息的seq */
				curr_seq = event_seq(mesgreq);
				last_seq = event_seq(info->sending);
				break;

			case NET_EV_DUMP_REP:	/* 如果上一次发送的是NET_EV_DUMP_REP，那么只能从持久化队列中获取最后seq */
				x_printf(I, "continue sync message after dump");
				curr_seq = event_seq(mesgreq);
				last_seq = xmq_fetch_position(info->consumer);
				last_seq = (last_seq < info->start_seq) ? info->start_seq - 1 : last_seq;
				break;

			default:/* NET_EV_DUMP_REQ 发送后不可能收到NET_EV_INCREMENT_REQ事件 */
				x_printf(W, "invalid type [%d] for last send event", info->sending->ev_type);
				delete_event(mesgreq);
				return -1;
		}
#if 0
		/* 检查最后一次发送的event seq 是不是当前seq 减1 */
		if (last_seq != curr_seq - 1) {
			x_printf(E, "current seq [%lu]is not followed by last seq [%lu]", curr_seq, last_seq);
			delete_event(mesgreq);
			return -1;
		}
#endif
	}

	mesgreq->incr.task_seq = curr_seq;

	// 获取持久化队列中的消息，发送给对端节点
	// 成功返回1; 无数据超时返回0; 错误返回-1
	int res = fetch_msg_to_send(info, mesgreq);

	if (res) {
		delete_event(mesgreq);
		res = (res == 1) ? 0 : res;
	}

	return res;
}

static int do_dump_request(porter_info_t *info, event_t *dumpreq)
{
	// 严重错误返回-1，清除信息当前线程退出
	if (dumpreq->ev_state == NET_EV_FATAL) {
		x_printf(E, "never recv a fatal NET_EV_DUMP_REQ event for this porter [%s]", info->peer);
		delete_event(dumpreq);
		return -1;
	}

	// NET_EV_DUMP_REQ消息数据长度检查，其数据保存的是需要导出BEASE的节点id
	if (!dumpreq->ev_size || (dumpreq->ev_size >= sizeof(dumpreq->id))) {
		x_printf(W, "invalid ev_data [%s] for NET_EV_DUMP_REQ event", dumpreq->ev_data);
		delete_event(dumpreq);
		return 0;
	}

	// 如果此时reserved不为空，那么必定是接收到了NET_EV_FAIL增量同步返回状态
	if (info->reserved) {
		if (info->reserved->ev_type == NET_EV_INCREMENT_REQ) {
			if (info->reserved->ev_state == NET_EV_FAIL) {
				x_printf(W, "Counld not dump!! There must be a sync error, wait for repair up!");
				delete_event(dumpreq);	// 由于已经接收到了NET_EV_FAIL事件，忽略该 NET_EV_DUMP_REQ 事件
				return 0;
			} else {
				x_printf(W, "last dump is not clear the reserved ?");
				delete_event(info->reserved);
			}
		} else {
			x_printf(W, "Invalid reserved event for it's type [%d]", info->reserved->ev_type);
			delete_event(info->reserved);
		}
	}

	if (info->sending) {
		int res = -1;
		switch (info->sending->ev_type)
		{
			case NET_EV_INCREMENT_REP:	// 如果上一次发送的是NET_EV_INCREMENT_REP，打印一条信息，后面会处理;
				x_printf(I, "we get dump request event after send message [%lu]", event_seq(info->sending));
				break;

			case NET_EV_DUMP_REQ:	// 如果上一次对该节点已经发送了NET_EV_DUMP_REQ，那么将本次NET_EV_DUMP_REQ缓存在todump队列中
				pipe_push_nolock(&info->todump, dumpreq);
				return 0;

			case NET_EV_DUMP_REP:	// 如果上一次已经发送了NET_EV_DUMP_REP事件，那么只需对请求节点重新再发一次NET_EV_DUMP_REP
				x_printf(I, "We just send response for dump, so only send for this porter [%s] more once!", dumpreq->id);

				strncpy(info->sending->id, dumpreq->id, sizeof(info->sending->id));
				res = send_event(info->transceiver, info->sending);

				if (res) {
					x_printf(E, "send_event error [%s]", event_error(res));
					delete_event(dumpreq);
					return -1;
				}

				return 0;

			default:
				break;
		}
	} else {
		x_printf(W, "We must get dump request event when no sending");
	}

	/* If send NET_EV_INCREMENT_REP last time, then we must waitting for it return. */
	if (!info->sending || (info->sending && (NET_EV_INCREMENT_REP == info->sending->ev_type))) {	// 如果上一次发送的是增量同步数据
		while (1) {
			event_t *next = pipe_pull(&info->events, 0);					// 等待上一个增量同步的返回

			if (next) {
				if (next->ev_type == NET_EV_INCREMENT_REQ) {	// 收到增量同步的返回
					info->reserved = next;			// 将增量同步最后返回状态缓存起来，等DUMP执行完了，再接着这个事件继续同步

					/* If client execute NET_EV_FAIL. */
					if (next->ev_state == NET_EV_FAIL) {	// 如果返回状态是NET_EV_FAIL
						x_printf(I, "last message [%lu] sync must failed, because received a WARN event [%lu]",
							event_seq(info->sending), event_seq(next));

						pipe_push_back(&info->events, dumpreq);	// 这里只是把NET_EV_DUMP_REQ放回到队列，由于现在reserved保存了一个NET_EV_FAIL事件
						return 0;				// 下一次再处理该事件
					} else if (next->ev_state == NET_EV_FATAL) {	// 如果返回NET_EV_FATAL状态
						delete_event(dumpreq);
						return -1;
					}

					break;
				} else if (next->ev_type == NET_EV_DUMP_REQ) {	// 如果又是一个NET_EV_DUMP_REQ，缓存到todump里面，继续等待增量同步的返回
					pipe_push_nolock(&info->todump, next);
					continue;
				} else {
					x_printf(W, "invalid next event type [%d]", next->ev_type);
					delete_event(next);
					continue;
				}
			} else {
				x_printf(E, "pipe_pull error [%s]", strerror(errno));
				delete_event(dumpreq);
				return -1;
			}
		}	// while(1)
	}

	// 生成一个新的 NET_EV_DUMP_REQ事件用于转发
	event_t *forward = make_dump_request(dumpreq);

	if (!forward) {
		delete_event(dumpreq);
		x_printf(E, "make_dump_request error");
		return -1;
	}

	// 缓存本次DUMP REQ 请求,在收到返回结果NET_EV_DUMP_REP事件的时候还要用
	pipe_push_nolock(&info->todump, dumpreq);

	// 转发NET_EV_DUMP_REQ 事件
	int res = send_event(info->transceiver, forward);

	if (res) {
		x_printf(E, "send_event error [%s]", event_error(res));
		return -1;
	}

	delete_event(info->sending);
	info->sending = forward;

	return 0;
}

static int do_dump_response(porter_info_t *info, event_t *dumprsp)
{
	/* 当执行NET_EV_DUMP操作的目标节点执行完成后,不会给服务器发送NET_EV_FATAL只会是NET_EV_FAIL. */
	if (dumprsp->ev_state == NET_EV_FATAL) {
		x_printf(W, "There must be a fatal error dump event from porter [%s]", dumprsp->id);
		delete_event(dumprsp);
		return 0;
	}

	// reserved 中必须保存一个最后一次接收到的增量同步返回
	if (!info->reserved) {
		x_printf(E, "the reserved event of porter [%s]is NULL", info->peer);
		delete_event(dumprsp);
		return -1;
	}

	// 上一次发送的一定是NET_EV_DUMP_REQ 事件
	if (!info->sending || (NET_EV_DUMP_REQ != info->sending->ev_type)) {
		x_printf(W, "we last send event type [%d]is not NET_EV_DUMP_REQ", info->sending->ev_type);
		delete_event(dumprsp);
		return 0;
	}

	int     count = 0;
	int     total = pipe_entrys(&info->todump);
	event_t *dumpreq = NULL;

	// 为todump中缓存的每个NET_EV_DUMP_REQ事件发送一个NET_EV_DUMP_REP事件响应
	while ((dumpreq = pipe_pull_nolock(&info->todump))) {
		// 避免相同节点重复DUMP
		if (look_up_porter(dumpreq->id)) {
			count++;
			delete_event(dumpreq);
			continue;
		}

		// 注册一个持久化队列消费者
		// xmq_consumer_t *consumer = xmq_register_as_consumer(info->consumer->owner_queue, dumpreq->id);
		int             res = 0;
		xmq_consumer_t  *consumer = xmq_get_consumer(info->consumer->xmq_ctx, dumpreq->id);

		if (!consumer) {
			x_printf(I, "xmq_get_consumer(%s) fail, The consumer name doesn't exist. we'll register it.", dumpreq->id);

			res = xmq_register_consumer(info->consumer->xmq_ctx, dumpreq->id);

			if (res != 0) {
				x_printf(W, "xmq_register_consumer(%s) fail.", dumpreq->id);
				delete_event(dumpreq);
				return -1;
			}

			consumer = xmq_get_consumer(info->consumer->xmq_ctx, dumpreq->id);
		}

		event_ctx_t     *transceiver = info->transceiver;
		uint64_t        start_seq = event_seq(info->reserved);	// 以最后一个增量同步的返回事件seq，作为新增节点的start seq

		// 仅仅创建一个数据结构，还没有启动线程
		porter_info_t *newporter = porter_create(dumpreq->id, consumer, transceiver, start_seq);

		if (!newporter) {
			x_printf(E, "porter_create failed for the porter [%s]", dumpreq->id);

			xmq_unregister_consumer(info->consumer->xmq_ctx, dumpreq->id);
			delete_event(dumpreq);
			continue;
		}

		strncpy(dumprsp->id, dumpreq->id, sizeof(dumprsp->id));
		res = send_event(info->transceiver, dumprsp);

		if (res) {
			x_printf(E, "send_event error [%s]", event_error(res));
			delete_event(dumpreq);
			porter_destroy(newporter);
			continue;
		}

		pthread_mutex_lock(&porters_list_lock);
		list_add_tail(&newporter->list, &porters_list_head);	// 添加到全局链表当中
		pthread_mutex_unlock(&porters_list_lock);

		delete_event(dumpreq);
		delete_event(info->sending);
		info->sending = dumprsp;

		count++;
	}

	// 将缓存的最后一个NET_EV_INCREMENT_REQ事件返回到events队列中，这个事件将会被重新获取执行
	pipe_push_back(&info->events, info->reserved);
	info->reserved = NULL;

	if (total != count) {
		x_printf(E, "total dump request is %d, and onely %d dump successfully", total, count);
	}

	return 0;
}

static void porter_error_term(porter_info_t *info)
{
	list_del(&info->list);
	porter_destroy(info);

	pthread_exit((void *)-1);
}

extern int porter_work_start(porter_info_t *info)
{
	pthread_t tid;

	int res = pthread_create(&tid, NULL, do_work, (void *)info);

	if (0 == res) {
		info->tid = tid;
	}

	return res;
}

