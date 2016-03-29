//
//  main.c
//  supex
//
//  Created by 周凯 on 15/12/12.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <stdio.h>
#include <zmq.h>
#include <ev.h>
#include "zmq_wrap.h"
#include "libmini.h"

#undef PKG_SIZE
#define PKG_SIZE 1024

struct iohandle
{
	char    name[32];
	enum
	{
		SOCK_ZMQ_BIND = 1,
		SOCK_ZMQ_CONN,
		SOCK_HTTP_BIND,
		SOCK_HTTP_CONN
	}       type;
	union
	{
		struct
		{
			void    *ctx;
			void    *skt;
			int     fd;
		}       zmq;
		int     http;
	};

	void    *usr;
};

static struct zmqframe *g_iocache = NULL;

static struct iohandle g_iohandle = {};

static struct ev_io     g_eio = {};
static struct ev_timer  g_etm = {};
static struct timeval   g_tv = {};

static void init_ctx(int argc, char **argv);

static void bind_push();

static void conn_push();

static void start_event();

static void stop_event();

static void echo_time(const char *msg);

static void io_cb(struct ev_loop *loop, struct ev_io *io, int event);

static void tm_cb(struct ev_loop *loop, struct ev_timer *tm, int event);

int main(int argc, char **argv)
{
	SetProgName(argv[0]);

	if (unlikely(argc != 3)) {
		x_perror("Usage %s <ip> <port>", g_ProgName);
		exit(EXIT_FAILURE);
	}

	init_ctx(argc, argv);

	if (strcmp(argv[1], "0.0.0.0") == 0) {
		/*bind*/
		bind_push();
	} else {
		conn_push();
	}

	return EXIT_SUCCESS;
}

static void init_ctx(int argc, char **argv)
{
	Rand();

	g_iohandle.zmq.ctx = zmq_ctx_new();
	AssertError(g_iohandle.zmq.ctx, ENOMEM);

	g_iohandle.zmq.skt = zmq_socket(g_iohandle.zmq.ctx, ZMQ_PULL);
	AssertRaise(g_iohandle.zmq.skt, EXCEPT_SYS);

	g_iohandle.usr = ev_default_loop(0);
	AssertError(g_iohandle.usr, ENOMEM);

	g_iocache = zmqframe_new(PKG_SIZE);
	AssertError(g_iocache, ENOMEM);

	snprintf(g_iohandle.name, sizeof(g_iohandle.name), "tcp://%s:%d", argv[1], atoi(argv[2]));
}

static void bind_push()
{
	int rc = 0;

	rc = zmq_bind(g_iohandle.zmq.skt, g_iohandle.name);
	RAISE_SYS_ERROR(rc);

	set_zmqopt(g_iohandle.zmq.skt, ZMQ_RCVHWM, 1000);
#ifdef IMMEDIATE
	set_zmqopt(g_iohandle.zmq.skt, ZMQ_IMMEDIATE, 1);
#endif

	g_iohandle.zmq.fd = get_zmqopt(g_iohandle.zmq.skt, ZMQ_FD);

	start_event();

	ev_run(g_iohandle.usr, 0);

	ev_loop_destroy(g_iohandle.usr);
}

static void conn_push()
{
	int rc = 0;

	rc = zmq_connect(g_iohandle.zmq.skt, g_iohandle.name);
	RAISE_SYS_ERROR(rc);

	/*发送缓冲为最小1000*/
	set_zmqopt(g_iohandle.zmq.skt, ZMQ_RCVHWM, 10);
#ifdef IMMEDIATE
	set_zmqopt(g_iohandle.zmq.skt, ZMQ_IMMEDIATE, 1);
#endif

	g_iohandle.zmq.fd = get_zmqopt(g_iohandle.zmq.skt, ZMQ_FD);

	start_event();

	ev_run(g_iohandle.usr, 0);

	ev_loop_destroy(g_iohandle.usr);
}

static void start_event()
{
	g_eio.data = &g_tv;
	g_etm.data = &g_tv;

	ev_io_init(&g_eio, io_cb, g_iohandle.zmq.fd, EV_READ);
	ev_io_start(g_iohandle.usr, &g_eio);

	ev_timer_init(&g_etm, tm_cb, 1, 0);
	ev_timer_start(g_iohandle.usr, &g_etm);

	gettimeofday(&g_tv, NULL);
}

static void stop_event()
{
	ev_io_stop(g_iohandle.usr, &g_eio);
	ev_timer_stop(g_iohandle.usr, &g_etm);
}

static void echo_time(const char *msg)
{
	struct timeval  tv = {};
	struct timeval  rc = {};

	gettimeofday(&tv, NULL);

	rc.tv_sec = tv.tv_sec - g_tv.tv_sec;
	rc.tv_usec = tv.tv_usec - g_tv.tv_usec;

	if (rc.tv_usec < 0) {
		rc.tv_sec -= 1;
		rc.tv_usec += 1000000;
	}

	fprintf(stderr, "\x1B[1;33m" "[%s] - [%d:%06d]" "\x1B[m" "\n", msg, (int)rc.tv_sec, (int)rc.tv_usec);
}

static void io_cb(struct ev_loop *loop, struct ev_io *io, int event)
{
#ifndef FEED_EVENT
	stop_event();
#endif

	echo_time("Read");

	//	bool flag = false;

	//	flag = check_zmqread(g_iohandle.zmq.skt, 1000);
	//	if (unlikely(!flag)) {
	//		x_printf(W, "Not readable.");
	//	} else {

	UNREFOBJ(g_iocache);

	int frames = read_zmqdata(g_iohandle.zmq.skt, g_iocache);

	if (likely(frames > 0)) {
#ifdef FEED_EVENT
		ev_feed_fd_event(loop, g_iohandle.zmq.fd, EV_READ);
#endif
		x_printf(I, "Read data : `%d` : `%d`", frames, (int)(g_iocache->offset - offsetof(struct zmqframe, data)));
	} else if (likely(frames == 0)) {
		x_printf(W, "Not readable.");
	} else {
		RAISE(EXCEPT_SYS);
	}

	//	}

#ifdef FEED_EVENT
	gettimeofday(&g_tv, NULL);
#else
	start_event();
#endif
}

static void tm_cb(struct ev_loop *loop, struct ev_timer *tm, int event)
{
	stop_event();

	echo_time("Timed out");

	start_event();
}

