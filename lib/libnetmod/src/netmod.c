/*
 * netmod.c
 *
 *  Created on: 2015/10/20
 *  Author: liubaoan@mirrtalk.com
 */

#include <zmq.h>
#include <pthread.h>

#include "netmod.h"
#include "libmini.h"

#define return_if_false(v, r)					       \
	do {							       \
		if (!(v)) {					       \
			x_printf(W, "Expression [ " #v " ] invalid."); \
			return (r);				       \
		}						       \
	} while (0)

static int _get_monitor_event(void *zmq_monitor, uint32_t *value, char *info, size_t size);

static void _print_monitor_event(void *zmq_monitor);

static void _set_net_property(void *zmq_skt);

evt_ctx_t *evt_ctx_init(enum node_type type, const char *uri_addr, const char *identity)
{
	assert(uri_addr != NULL);
	assert((type == SOCK_SERVER) ||
		((type == SOCK_CLIENT) && identity));

	evt_ctx_t *ectx = (evt_ctx_t *)malloc(sizeof(evt_ctx_t));

	if (!ectx) {
		goto INIT_FAIL;
	}

	ectx->type = type;

	/* 全局ZeroMQ类型的Context. */
	void *zctx = zmq_ctx_new();
	ectx->zmq_ctx = zctx;

	if (!zctx) {
		goto INIT_FAIL;
	}

	/* 创建事件镜像SOCKET */
	void *monitor = zmq_socket(zctx, ZMQ_PAIR);
	ectx->zmq_monitor = monitor;

	if (!monitor) {
		goto INIT_FAIL;
	}

	if (zmq_connect(monitor, "inproc://monitor")) {
		goto INIT_FAIL;
	}

	/* 创建事件监听SOCKET */
	void *sockfd = zmq_socket(zctx, (type == SOCK_SERVER) ? ZMQ_ROUTER : ZMQ_DEALER);
	ectx->zmq_socket = sockfd;

	if (!sockfd) {
		goto INIT_FAIL;
	}

	/* 注册一个事件监听回调机制 */
	if (zmq_socket_monitor(sockfd, "inproc://monitor", ZMQ_EVENT_ALL)) {
		goto INIT_FAIL;
	}

	/* SET 0MQ network property. */
	_set_net_property(sockfd);

	if (type == SOCK_SERVER) {
		if (zmq_bind(sockfd, uri_addr) != 0) {
			_print_monitor_event(monitor);
			x_printf(E, "evt_ctx_init fail. Error-%s.", zmq_strerror(errno));
			goto INIT_FAIL;
		}
	} else {
		/* SET client's identity. */
		zmq_setsockopt(sockfd, ZMQ_IDENTITY, identity, strlen(identity) + 1);

		/*
		 * Only when connect to server okay, we send data to it.
		 * ZMQ_IMMEDIATE used only for (REQ|PUSH|DEALER).
		 */
		int optval = 1;
		zmq_setsockopt(sockfd, ZMQ_IMMEDIATE, (void *)&optval, sizeof(optval));

		/* SET reconnect time interval. */
		optval = 200;	// 0.2s.
		zmq_setsockopt(sockfd, ZMQ_RECONNECT_IVL, (void *)&optval, sizeof(optval));

		/* SET the maxminum reconnect time interval. */
		optval = 10 * 1000;	// 10s.
		zmq_setsockopt(sockfd, ZMQ_RECONNECT_IVL_MAX, (void *)&optval, sizeof(optval));

		if (zmq_connect(sockfd, uri_addr) != 0) {
			_print_monitor_event(monitor);
			x_printf(E, "evt_ctx_init fail. Error-%s.", zmq_strerror(errno));
			goto INIT_FAIL;
		}
	}

	qlist_init(&ectx->qrecv);
	qlist_init(&ectx->qsend);

	return ectx;

INIT_FAIL:

	if (ectx) {
		if (ectx->zmq_socket) {
			zmq_close(ectx->zmq_socket);
		}

		if (ectx->zmq_ctx) {
			zmq_ctx_destroy(ectx->zmq_ctx);
		}

		free(ectx);
	}

	return NULL;
}

int evt_ctx_destroy(evt_ctx_t *evt_ctx)
{
	assert(evt_ctx != NULL);

	if (zmq_close(evt_ctx->zmq_socket)) {
		return errno;
	}

	if (zmq_close(evt_ctx->zmq_monitor)) {
		return errno;
	}

	if (zmq_ctx_destroy(evt_ctx->zmq_ctx)) {
		return errno;
	}

	qlist_free(&evt_ctx->qrecv);
	qlist_free(&evt_ctx->qsend);

	free(evt_ctx);

	return 0;
}

evt_t *evt_new_by_size(size_t size)
{
	evt_t *ev = (evt_t *)calloc(1, evt_head_size() + size);

	if (ev) {
		ev->ev_size = size;
	}

	return ev;
}

evt_t *copy_evt(const evt_t *evt)
{
	assert(evt != NULL);

	evt_t *ev_des = evt_new_by_size(evt_body_size(evt));

	if (ev_des) {
		memcpy(ev_des, evt, evt_total_size(evt));
	}

	return ev_des;
}

void free_evt(evt_t *evt)
{
	if (evt) {
		free(evt); evt = NULL;
	}
}

evt_t *recv_evt(evt_ctx_t *evt_ctx)
{
	evt_t   *evt = NULL;
	QITEM   *item = qlist_pull(&evt_ctx->qrecv);

	if (item) {
		evt = item->data;
		qitem_free(item);
	}

	return evt;
}

int send_evt(evt_ctx_t *evt_ctx, evt_t *evt)
{
	return_if_false((evt_ctx && evt), -1);

	QITEM *item = qitem_init(NULL);
	assert(item);
	item->data = evt;

	qlist_push(&evt_ctx->qsend, item);

	return 0;
}

void *work_evt(evt_ctx_t *evt_ctx)
{
	int     ok = 0;
	char    id[IDENTITY_SIZE];
	evt_t   ev_head, *evt;
	char    *ev_body;
	int     len, res = 0;

	zmq_pollitem_t items [2];

	items[0].socket = evt_ctx->zmq_socket;
	items[0].fd = 0;
	items[0].events = ZMQ_POLLIN;
	items[0].revents = 0;
	items[1].socket = evt_ctx->zmq_socket;
	items[1].fd = 0;
	items[1].events = ZMQ_POLLOUT;
	items[1].revents = 0;

	while (1) {
		ok = zmq_poll(items, 2, 0);

		if (ok == -1) {
			x_printf(E, "WARN: thrd_netCenter:-> zmq_poll: fail. Error:%s\n", zmq_strerror(errno));
			_print_monitor_event(evt_ctx->zmq_monitor);
			return NULL;
		} else if (ok == 0) {
			/* Now, The poll I/O have no task to handle.*/
			continue;
		}

		if (ok > 0) {
			if (items[0].revents & ZMQ_POLLIN) {
				/* ================= 接收所有请求 ================== */
				void *skt = items[0].socket;

				/* Receive Client Identity. If current socket is Server. */
				if (evt_ctx->type == SOCK_SERVER) {
					res = zmq_recv(skt, id, sizeof(id), 0);
					assert(res != -1);
				}

				/* 1. Getting Event-Package Head. */
				len = evt_head_size();
				res = zmq_recv(skt, &ev_head, len, 0);

				if (res != len) {
					x_printf(E, "_zmq_recv(ev_head:%d) bytes fail. Error:%s\n",
						len, zmq_strerror(errno));
					_print_monitor_event(evt_ctx->zmq_monitor);
				}

				assert(res != -1);

				/* Copy the package head to evt. */
				len = evt_body_size(&ev_head);
				evt = evt_new_by_size(len);
				assert(evt);
				memcpy(evt, &ev_head, evt_head_size());

				/* 2. Receiving Event-Package Body, If there is. */
				char *ev_body = evt->ev_data;

				if (len > 0) {
					res = zmq_recv(skt, ev_body, len, 0);

					if (res != len) {
						x_printf(E, "_zmq_recv(ev_body:%d) bytes fail. Error:%s\n",
							len, zmq_strerror(errno));
						_print_monitor_event(evt_ctx->zmq_monitor);
					}

					assert(res != -1);
				}

				/* 3. Add received Event-Package to Receive-Queue. */
				QITEM *item = qitem_init(NULL);
				item->data = evt;
				qlist_push(&evt_ctx->qrecv, item);
			}

			if (items[1].revents & ZMQ_POLLOUT) {
				void *skt = items[1].socket;
				/* =======================SEND EVENT=======================*/
				QITEM *item = qlist_pull(&evt_ctx->qsend);

				if (!item) {
					/* When the send module has no task to handle, sleep(0.01s).*/
					usleep(10000);
					continue;
				}

				evt_t *evt = item->data;
				qitem_free(item);

				/* 1. Send Client Identity, If current socket is Server. */
				if (evt_ctx->type == SOCK_SERVER) {
					len = strlen(evt->id) + 1;
					res = zmq_send(skt, evt->id, len, ZMQ_SNDMORE);

					if (res != len) {
						x_printf(E, "zmq_send(id:%d) bytes fail. Error:%s\n",
							len, zmq_strerror(errno));
						_print_monitor_event(evt_ctx->zmq_monitor);
					}

					assert(res != -1);
				}

				/* 2. Send Event-Package Head. */
				len = evt_head_size();
				res = (evt_body_size(evt) > 0) ? ZMQ_SNDMORE : 0;
				res = zmq_send(skt, evt, len, res);

				if (res != len) {
					x_printf(E, "zmq_send(ev_head:%d) bytes fail. Error:%s\n",
						len, zmq_strerror(errno));
					_print_monitor_event(evt_ctx->zmq_monitor);
				}

				assert(res != -1);

				/*
				 * 3. If event body isn't null,
				 * Such as: "DUMP CMD-> The body is destination node ID."
				 */
				len = evt_body_size(evt);

				if (len > 0) {
					res = zmq_send(skt, evt->ev_data, len, 0);

					if (res != len) {
						x_printf(E, "zmq_send(ev_body:%d) bytes fail. Error:%s\n",
							len, zmq_strerror(errno));
						_print_monitor_event(evt_ctx->zmq_monitor);
					}

					assert(res != -1);
				}

				/* 4. Destroy and free the memory leak. */
				free_evt(evt);
			}
		}
	}

	return NULL;
}

static void _set_net_property(void *sockfd)
{
	int optval = 0;

	// SET the maxminum of sending queue's length
	optval = 500;
	zmq_setsockopt(sockfd, ZMQ_SNDHWM, (void *)&optval, sizeof(optval));

	// SET the maxminum of recving queue's length
	optval = 500;
	zmq_setsockopt(sockfd, ZMQ_RCVHWM, (void *)&optval, sizeof(optval));

	// SET the waitting time, when socket is closed.
	optval = 5000;	// 5s.
	zmq_setsockopt(sockfd, ZMQ_LINGER, (void *)&optval, sizeof(optval));

	// SET the max-waitting time before return EAGAIN, for send timeout.
	optval = 5000;	// 5s.
	zmq_setsockopt(sockfd, ZMQ_SNDTIMEO, (void *)&optval, sizeof(optval));
}

static int _get_monitor_event(void *zmq_monitor, uint32_t *value, char *info, size_t size)
{
	return_if_false((zmq_monitor && value && info), -1);

	zmq_msg_t msg;

	// 第一帧: 前2个字节(16bit)包含了事件的ID,后4个字节(32bit)事件值
	zmq_msg_init(&msg);

	if (zmq_msg_recv(&msg, zmq_monitor, 0)) {
		x_printf(W, "zmq_msg_recv() First Frame fail. Error-%s", zmq_strerror(errno));
		return -1;
	}

	assert(zmq_msg_more(&msg));

	uint8_t *data = (uint8_t *)zmq_msg_data(&msg);
	// GET event ID. the first 16bit.
	uint16_t event = *(uint16_t *)data;
	// GET event value. the left 32bit.
	*value = *(uint32_t *)(data + 2);

	// 第二帧: 包含了事件的TCP/IPC类型的地址
	zmq_msg_init(&msg);

	if (zmq_msg_recv(&msg, zmq_monitor, 0)) {
		x_printf(W, "zmq_msg_recv() Second Frame fail. Error-%s", zmq_strerror(errno));
		return -1;
	}

	assert(!zmq_msg_more(&msg));

	memset(info, 0, size);
	int     get = zmq_msg_size(&msg);
	int     len = MIN(get, size);
	memcpy(info, zmq_msg_data(&msg), len);

	zmq_msg_close(&msg);

	return event;
}

static void _print_monitor_event(void *zmq_monitor)
{
	assert(zmq_monitor);

	uint32_t        ev_val = -1;
	char            addr[128] = { 0 };

	switch (_get_monitor_event(zmq_monitor, &ev_val, addr, sizeof(addr) - 1))
	{
		case ZMQ_EVENT_CONNECTED:
			x_printf(I, "New connect was create succeed, address-%s socket fd-%u", addr, ev_val);
			break;

		case ZMQ_EVENT_CONNECT_DELAYED:
			x_printf(I, "Trying connect was delayed, and trying again.");
			break;

		case ZMQ_EVENT_CONNECT_RETRIED:
			x_printf(I, "Trying connect was catched by reconnect-timer, and will try again after %u ms. address-%s", ev_val, addr);
			break;

		case ZMQ_EVENT_LISTENING:
			x_printf(I, "The port was bind succeed, start listening, address-%s socket fd-%u", addr, ev_val);
			break;

		case ZMQ_EVENT_BIND_FAILED:
			x_printf(S, "Cann't bind the port, address-%s Error-%s", addr, zmq_strerror(ev_val));
			break;

		case ZMQ_EVENT_ACCEPTED:
			x_printf(I, "Server accept a new client, address-%s the accepted socket fd-%u", addr, ev_val);
			break;

		case ZMQ_EVENT_ACCEPT_FAILED:
			x_printf(E, "Server cann't accept the client connect, address-%s Error-%s", addr, zmq_strerror(ev_val));
			break;

		case ZMQ_EVENT_CLOSED:
			x_printf(I, "The socket fd-%u was closed, address-%s", ev_val, addr);
			break;

		case ZMQ_EVENT_CLOSE_FAILED:
			x_printf(E, "The socket fd cann't be freed by OS, address-%s Error-%s", addr, zmq_strerror(ev_val));
			break;

		case ZMQ_EVENT_DISCONNECTED:
			x_printf(F, "The session of socket fd-%u was broken, address-%s", ev_val, addr);
			break;

		default:
			x_printf(W, "Invalid ZeroMQ event type.");
			break;
	}
}

void print_evt(const evt_t *ev)
{
	assert(ev != NULL);

	x_printf(D, " id ->'%s'\n ev_type ->'%s'\n ev_state->'%s'\n task_seq-> %d\n ev_size -> %d\n ev_data ->'%s'\n\n", \
		ev->id,													 \
		ev->ev_type == NET_EV_DUMP_REQ ? "NET_EV_DUMP_REQ" :							 \
		(ev->ev_type == NET_EV_DUMP_REP ? "NET_EV_DUMP_REP" :							 \
		(ev->ev_type == NET_EV_INCREMENT_REQ ? "NET_EV_INCREMENT_REQ" : "NET_EV_INCREMENT_REP")),		 \
		ev->ev_state == NET_EV_SUCC ? "NET_EV_SUCC" :								 \
		(ev->ev_state == NET_EV_NONE ? "NET_EV_NONE" :								 \
		(ev->ev_state == NET_EV_FAIL ? "NET_EV_FAIL" : "NET_EV_FATAL")),					 \
		ev->incr.task_seq,											 \
		ev->ev_size,												 \
		ev->ev_size ? ev->ev_data : ""										 \
		);
}

const char *evt_error(int error)
{
	return zmq_strerror(error);
}

