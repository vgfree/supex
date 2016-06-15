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

static void *thrd_netCenter(void *args);

static int get_monitor_event(void *skt_moniter, int *value, char *address);

static void print_monitor_event(void *zmq_monitor);

static void set_net_property(void *zmq_skt);

static char g_systime[64];

static queue_ctx_t *queue_ctx_init();

static void queue_push(queue_ctx_t *qctx, event_t *ev);

static event_t *queue_pull(queue_ctx_t *qctx, int timeout);

static size_t queue_size(queue_ctx_t *qctx);

static void queue_delete_all_events(queue_ctx_t *qctx);

static void queue_ctx_destroy(queue_ctx_t *qctx);

event_ctx_t *event_ctx_init(int *error, int node_type, const char *uri_addr, const char *identity)
{
	assert(error != NULL);
	assert(uri_addr != NULL);
	assert(node_type == SOCK_CLIENT || node_type == SOCK_SERVER);

	if ((node_type == SOCK_CLIENT) && (identity == NULL)) {
		*error = EINVAL;
		return NULL;
	}

	void            *sockfd;
	queue_ctx_t     *qrecv, *qsend;

	// 全局ZeroMQ类型的Context.
	void *zctx = zmq_ctx_new();
	return_if_false(zctx != NULL, NULL);

	// 创建事件监听SOCKET
	void *monitor = zmq_socket(zctx, ZMQ_PAIR);
	return_if_false(monitor != NULL, NULL);

	int res = zmq_connect(monitor, "inproc://monitor");
	return_if_false(res != -1, NULL);

	event_ctx_t *ectx = (event_ctx_t *)malloc(sizeof(event_ctx_t));
	return_if_false(ectx != NULL, NULL);

	if (node_type == SOCK_SERVER) {
		sockfd = zmq_socket(zctx, ZMQ_ROUTER);

		if (!sockfd) {
			goto INIT_FAIL;
		}

		// 注册一个 ROUTER 类型的事件监听回调机制
		if (zmq_socket_monitor(sockfd, "inproc://monitor", ZMQ_EVENT_ALL)) {
			goto ZMQ_FAIL;
		}

		// SET 0MQ network property.
		set_net_property(sockfd);

		if (zmq_bind(sockfd, uri_addr) != 0) {
			print_monitor_event(monitor);
			goto ZMQ_FAIL;
		}

		ectx->node_type = SOCK_SERVER;
	} else {
		sockfd = zmq_socket(zctx, ZMQ_DEALER);

		if (!sockfd) {
			goto INIT_FAIL;
		}

		// 注册一个 DEALER 类型的事件监听回调机制
		if (zmq_socket_monitor(sockfd, "inproc://monitor", ZMQ_EVENT_ALL)) {
			goto ZMQ_FAIL;
		}

		// SET 0MQ network property.
		set_net_property(sockfd);

		// SET client's identity.
		zmq_setsockopt(sockfd, ZMQ_IDENTITY, identity, strlen(identity) + 1);

		// Only when connect to server okay, we send data to it.
		// ZMQ_IMMEDIATE used only for (REQ|PUSH|DEALER).
		int optval = 1;
		zmq_setsockopt(sockfd, ZMQ_IMMEDIATE, (void *)&optval, sizeof(optval));

		// SET reconnect time interval.
		optval = 200;	// 0.2s.
		zmq_setsockopt(sockfd, ZMQ_RECONNECT_IVL, (void *)&optval, sizeof(optval));

		// SET the maxminum reconnect time interval.
		optval = 10 * 1000;	// 10s.
		zmq_setsockopt(sockfd, ZMQ_RECONNECT_IVL_MAX, (void *)&optval, sizeof(optval));

		if (zmq_connect(sockfd, uri_addr) != 0) {
			print_monitor_event(monitor);
			goto ZMQ_FAIL;
		}

		ectx->node_type = SOCK_CLIENT;
	}

	qrecv = queue_ctx_init();
	qsend = queue_ctx_init();

	assert(qrecv && qsend);

	ectx->zmq_ctx = zctx;
	ectx->zmq_socket = sockfd;
	ectx->zmq_monitor = monitor;
	ectx->recv_queue = qrecv;
	ectx->send_queue = qsend;

	/* Startup the thread of Network Center to Recv & Send data. */
	pthread_t       thrd;
	pthread_attr_t  attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	res = pthread_create(&thrd, &attr, thrd_netCenter, ectx);
	usleep(10 * 1000);
	return_if_false(res == 0, NULL);

	memcpy(&ectx->thrd_kill, &thrd, sizeof(pthread_t));

	return ectx;

ZMQ_FAIL:
	x_printf(E, "event_ctx_init fail. Error-%s.", zmq_strerror(errno));
	zmq_close(sockfd);
INIT_FAIL:
	free(ectx);
	zmq_ctx_destroy(zctx);
	*error = errno;
	return NULL;
}

event_t *event_new_size(size_t size)
{
	event_t *ev = (event_t *)calloc(1, event_head_size() + size);

	if (ev) {
		ev->ev_size = size;
	}

	return ev;
}

event_t *event_dup(const event_t *ev)
{
	assert(ev != NULL);

	event_t *ev_des = event_new_size(event_body_size(ev));

	if (ev_des) {
		memcpy(ev_des, ev, event_total_size(ev));
	}

	return ev_des;
}

void delete_event(event_t *ev)
{
	if (ev) {
		free(ev); ev = NULL;
	}
}

event_t *recv_event(const event_ctx_t *ev_ctx, long timeout)
{
	return_if_false((ev_ctx && ev_ctx->recv_queue && timeout >= -1), NULL);

	return queue_pull(ev_ctx->recv_queue, timeout);
}

int send_event(const event_ctx_t *ev_ctx, const event_t *ev)
{
	return_if_false((ev_ctx && ev_ctx->send_queue && ev), -1);

	event_t *ev_des = event_dup(ev);
	return_if_false(ev_des, -1);

	queue_push(ev_ctx->send_queue, ev_des);

	return 0;
}

void *thrd_netCenter(void *args)
{
	event_ctx_t *ev_ctx = (event_ctx_t *)args;

	int             res, len, tasks = 0;
	char            id[IDENTITY_SIZE];
	event_t         ev, *ev_des;
	zmq_pollitem_t  items = { ev_ctx->zmq_socket, 0, ZMQ_POLLIN, 0 };

	while (1) {
		/* ================= 接收所有请求 ================== */
		res = zmq_poll(&items, 1, 0);

		if ((res > 0) && (items.revents & ZMQ_POLLIN)) {

			++tasks; /* Flag, to show how many tasks to handle. */

			// printf("ZMQ_POLL RECV F_NETWORK: %s.\n", _systime());
			/* Receive Client Identity. If current socket is Server. */
			if (ev_ctx->node_type == SOCK_SERVER) {
RECV_ID:
				res = zmq_recv(ev_ctx->zmq_socket, id, sizeof(id), 0);

				if ((res == -1) && (errno == EINTR)) {
					goto RECV_ID;
				}
			}

			/* 1. Getting Event-Package Head. */
			len = event_head_size();
RECV_HEAD:
			res = zmq_recv(ev_ctx->zmq_socket, &ev, len, 0);

			if (res != len) {
				if ((res == -1) && (errno == EINTR)) {
					goto RECV_HEAD;
				}

				x_printf(E, "thrd_netCenter:->_zmq_recv(ev_head:%d) bytes fail. Error:%s\n", len, zmq_strerror(errno));
				print_monitor_event(ev_ctx->zmq_monitor);
				goto SEND_PKG;
			}

			len = event_body_size(&ev);
			ev_des = event_new_size(len);

			if (!ev_des) {
				x_printf(E, "thrd_netCenter:->_alloc(event_t:%d) bytes fail. Error:%s\n", len, strerror(errno));
				goto SEND_PKG;
			}

			/* Copy the package head to ev_des. */
			memcpy(ev_des, &ev, event_head_size());

			/* 2. Receiving Event-Package Body, If there is. */
			if (len > 0) {
RECV_BODY:
				res = zmq_recv(ev_ctx->zmq_socket, ev_des->ev_data, len, 0);

				if (res != len) {
					if ((res == -1) && (errno == EINTR)) {
						goto RECV_BODY;
					}

					delete_event(ev_des);
					x_printf(E, "thrd_netCenter:->_zmq_recv(ev_body:%d) bytes fail. Error:%s\n", len, zmq_strerror(errno));
					print_monitor_event(ev_ctx->zmq_monitor);
					goto SEND_PKG;
				}
			}

			/* 3. Add received Event-Package to Receive-Queue. */
			queue_push(ev_ctx->recv_queue, ev_des);
		} else if (res == -1) {
			x_printf(E, "WARN: thrd_netCenter:-> zmq_poll: fail. Error:%s\n", zmq_strerror(errno));
			print_monitor_event(ev_ctx->zmq_monitor);
		} else if (res == 0) {
			tasks = -1; /* Now, The poll I/O have no task to handle.*/
		}

		/* =======================SEND EVENT=======================*/
SEND_PKG:
		ev_des = queue_pull(ev_ctx->send_queue, 0);//TODO:

		if (!ev_des) {
			if (tasks == -1) {
				/* When the receive and send module has no task to handle, sleep(0.01s).*/
				usleep(10000);
			}
			continue;
		}else{
			++tasks;
		}

		/* 1. Send Client Identity, If current socket is Server. */
		if (ev_ctx->node_type == SOCK_SERVER) {
			len = strlen(ev_des->id) + 1;
SEND_ID:
			res = zmq_send(ev_ctx->zmq_socket, ev_des->id, len, ZMQ_SNDMORE);

			if (res != len) {
				if ((res == -1) && (errno == EINTR)) {
					goto SEND_ID;
				}

				x_printf(E, "thrd_netCenter:->zmq_send(id:%d) bytes fail. Error:%s\n", len, zmq_strerror(errno));
				print_monitor_event(ev_ctx->zmq_monitor);
				goto SEND_FAIL;
			}
		}

		/* Send Event-Package Head. */
		len = event_head_size();
		res = (event_body_size(ev_des) > 0) ? ZMQ_SNDMORE : 0;
SEND_HEAD:
		res = zmq_send(ev_ctx->zmq_socket, ev_des, len, res);

		if (res != len) {
			if ((res == -1) && (errno == EINTR)) {
				goto SEND_HEAD;
			}

			x_printf(E, "thrd_netCenter:->zmq_send(ev_head:%d) bytes fail. Error:%s\n", len, zmq_strerror(errno));
			print_monitor_event(ev_ctx->zmq_monitor);
			goto SEND_FAIL;
		}

		/* If event body isn't null, Such as: "DUMP CMD-> The body is destination node ID."*/
		len = event_body_size(ev_des);

		if (len > 0) {
SEND_BODY:
			res = zmq_send(ev_ctx->zmq_socket, ev_des->ev_data, len, 0);

			if (res != len) {
				if ((res == -1) && (errno == EINTR)) {
					goto SEND_BODY;
				}

				x_printf(E, "thrd_netCenter:->zmq_send(ev_body:%d) bytes fail. Error:%s\n", len, zmq_strerror(errno));
				print_monitor_event(ev_ctx->zmq_monitor);
				goto SEND_FAIL;
			}
		}

		/* Destroy and free the memory leak. */
SEND_FAIL:
		// printf("AFTER ZMQ_SEND: %s.\n", _systime());
		delete_event(ev_des);
	}	/* end while. */

	return NULL;
}

static void set_net_property(void *sockfd)
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

static int get_monitor_event(void *zmq_monitor, int *value, char *address)
{
	return_if_false((zmq_monitor && value && address), -1);

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

	memcpy(address, zmq_msg_data(&msg), zmq_msg_size(&msg));
	address[zmq_msg_size(&msg)] = '\0';

	zmq_msg_close(&msg);

	return event;
}

static void print_monitor_event(void *zmq_monitor)
{
	assert(zmq_monitor);

	int             ev_val = -1;
	static char     addr[128] = { 0 };

	switch (get_monitor_event(zmq_monitor, &ev_val, addr))
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

void print_event(const event_t *ev)
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

const char *event_error(int error)
{
	return zmq_strerror(error);
}

int event_ctx_destroy(event_ctx_t *ev_ctx)
{
	assert(ev_ctx != NULL);

	// Send SIGQUIT signal to thrd_netCenter.
	pthread_kill(ev_ctx->thrd_kill, SIGQUIT);
	sleep(1);

	if (zmq_close(ev_ctx->zmq_socket)) {
		return errno;
	}

	if (zmq_close(ev_ctx->zmq_monitor)) {
		return errno;
	}

	if (zmq_ctx_destroy(ev_ctx->zmq_ctx)) {
		return errno;
	}

	queue_ctx_destroy(ev_ctx->recv_queue);
	queue_ctx_destroy(ev_ctx->send_queue);

	free(ev_ctx);

	return 0;
}

static queue_ctx_t *queue_ctx_init()
{
	queue_ctx_t *qctx = (queue_ctx_t *)malloc(sizeof(queue_ctx_t));

	if (qctx) {
		list_init(&qctx->head);
		qctx->size = 0;

		if (pthread_cond_init(&qctx->cond, NULL)) {
			goto QINIT_FAIL;
		}

		if (pthread_mutex_init(&qctx->lock, NULL)) {
			goto QINIT_FAIL;
		}
	}

	return qctx;

QINIT_FAIL:
	free(qctx);
	return NULL;
}

static void queue_push(queue_ctx_t *qctx, event_t *ev)
{
	pthread_mutex_lock(&qctx->lock);

	list_add_tail(&ev->list, &qctx->head);
	++(qctx->size);

	if (qctx->size == 1) {
		// 唤醒其他所有读取此队列数据的线程,开始读取数据;
		pthread_cond_broadcast(&qctx->cond);
	}

	pthread_mutex_unlock(&qctx->lock);
}

static event_t *queue_pull(queue_ctx_t *qctx, int timeout)
{
	pthread_mutex_lock(&qctx->lock);

	if (qctx->size == 0) {
		int res = -1;
		switch (timeout)
		{
			case  0:
				pthread_mutex_unlock(&qctx->lock);
				return NULL;

			case -1:

				while (qctx->size == 0) {
					res = pthread_cond_wait(&qctx->cond, &qctx->lock);

					if (res != 0) {
						pthread_mutex_unlock(&qctx->lock);
						x_printf(E, "pthread_cond_wait Execute fail. Error-%s.", strerror(res));
						return NULL;
					}
				}

				break;

			default:

				while (qctx->size == 0) {
					struct timespec ts;
					clock_gettime(CLOCK_REALTIME, &ts);
					ts.tv_nsec += (timeout * 1000000);
					res = ts.tv_nsec / 1000000000;

					if (res > 0) {
						ts.tv_nsec %= 1000000000;
						ts.tv_sec += res;
					}

					res = pthread_cond_timedwait(&qctx->cond, &qctx->lock, &ts);

					if (res != 0) {
						pthread_mutex_unlock(&qctx->lock);

						if (res != ETIMEDOUT) {
							x_printf(E, "pthread_cond_timedwait Execute fail. Error-%s.", strerror(res));
						}

						return NULL;
					}
				}
		}
	}

	assert(qctx->size > 0);

	// Detach the last node from the event_t's list.
	xlist_t *last = NULL;
	list_del_tail(&last, &qctx->head);
	assert(last != NULL);

	--(qctx->size);

	pthread_mutex_unlock(&qctx->lock);

	return (event_t *)container_of(last, event_t, list);
}

static size_t queue_size(queue_ctx_t *qctx)
{
	size_t qsize = 0;

	pthread_mutex_lock(&qctx->lock);
	qsize = qctx->size;
	pthread_mutex_unlock(&qctx->lock);

	return qsize;
}

static void queue_delete_all_events(queue_ctx_t *qctx)
{
	pthread_mutex_lock(&qctx->lock);

	list_del_all_entrys(&qctx->head, event_t, list);

	pthread_mutex_unlock(&qctx->lock);
}

static void queue_ctx_destroy(queue_ctx_t *qctx)
{
	if (qctx->size > 0) {
		queue_delete_all_events(qctx);
	}

	pthread_cond_destroy(&qctx->cond);
	pthread_mutex_destroy(&qctx->lock);

	free(qctx);
}

const char *_systime()
{
	struct timeval t;

	gettimeofday(&t, NULL);
	struct tm *stm = localtime(&t.tv_sec);
	memset(g_systime, 0, sizeof(g_systime));
	sprintf(g_systime, /*"%04d."*/ "%02d/%02d %02d:%02d:%02d.%06u",	\
		/*stm->tm_year+1900,*/					\
		stm->tm_mon + 1,					\
		stm->tm_mday,						\
		stm->tm_hour,						\
		stm->tm_min,						\
		stm->tm_sec,						\
		(size_t)(t.tv_usec)
		);
	return g_systime;
}

