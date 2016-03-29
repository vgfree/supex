//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/9/17.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <stdio.h>
#include <ev.h>
#include <zmq.h>
#include "libmini.h"

struct zmqaction
{
	void    *skt;
	char    *buff;
	size_t  size;
	void    *usr;
	void    (*rcb)(char *, size_t, void *);
	void    (*wcb)(char *, size_t *, void *);
};

static void setzmqimmediate(void *skt)
{
	int     flag = 0;
	int     opt = 1;
	size_t  vsize = sizeof(opt);

	flag = zmq_setsockopt(skt, ZMQ_IMMEDIATE, &opt, vsize);
	RAISE_SYS_ERROR(flag);
}

static void zmqrcb(char *buff, size_t size, void *usr);

static void zmqwcb(char *buff, size_t *size, void *usr);

static void zmqcallback(struct ev_loop *loop, struct ev_io *watcher, int event);

int main(int argc, char **argv)
{
	void                    *ctx = NULL;
	struct ev_loop          *loop = NULL;
	char                    buff[MAX_LINE_SIZE] = {};
	char                    ipaddr[32];
	struct zmqaction        zmqrcv = {};
	struct zmqaction        zmqsnd = {};

	if (unlikely(argc != 5)) {
		x_printf(E, "Usage %s <read_ip> <#read_port> <write_ip> <#write_port>", argv[0]);
		return EXIT_FAILURE;
	}

	Rand();

	TRY
	{
		int     flag = 0;
		int     fd = 0;
		size_t  optsize = 0;

		struct ev_io    rcvwatcher = {};
		struct ev_io    sndwatcher = {};

		loop = ev_default_loop(EVFLAG_AUTO);
		assert(loop);

		ctx = zmq_ctx_new();
		assert(ctx);

		x_printf(D, "initialize pull-socket");
		zmqrcv.skt = zmq_socket(ctx, ZMQ_PULL);
		AssertRaise(zmqrcv.skt, EXCEPT_SYS);

		snprintf(ipaddr, sizeof(ipaddr), "tcp://%s:%d", argv[1], atoi(argv[2]));
		flag = zmq_connect(zmqrcv.skt, ipaddr);
		RAISE_SYS_ERROR(flag);

		setzmqimmediate(zmqrcv.skt);

		optsize = sizeof(fd);
		flag = zmq_getsockopt(zmqrcv.skt, ZMQ_FD, &fd, &optsize);
		RAISE_SYS_ERROR(flag);

		x_printf(D, "initialize read event for pull-socket");

		zmqrcv.buff = buff;
		zmqrcv.size = sizeof(buff);
		zmqrcv.usr = NULL;
		zmqrcv.rcb = zmqrcb;

		rcvwatcher.data = &zmqrcv;
		ev_io_init(&rcvwatcher, zmqcallback, fd, EV_READ);
		ev_io_start(loop, &rcvwatcher);

		x_printf(D, "initialize push-socket");
		zmqsnd.skt = zmq_socket(ctx, ZMQ_PUSH);
		AssertRaise(zmqsnd.skt, EXCEPT_SYS);

		/*其他选项*/
		setzmqimmediate(zmqsnd.skt);

		snprintf(ipaddr, sizeof(ipaddr), "tcp://%s:%d", argv[3], atoi(argv[4]));
		flag = zmq_connect(zmqsnd.skt, ipaddr);
		RAISE_SYS_ERROR(flag);

		optsize = sizeof(fd);
		flag = zmq_getsockopt(zmqsnd.skt, ZMQ_FD, &fd, &optsize);
		RAISE_SYS_ERROR(flag);

		x_printf(D, "initialize write event for push-socket");

		zmqsnd.buff = buff;
		zmqsnd.size = sizeof(buff);
		zmqsnd.usr = NULL;
		zmqsnd.wcb = zmqwcb;

		sndwatcher.data = &zmqsnd;
		ev_io_init(&sndwatcher, zmqcallback, fd, EV_WRITE);
		ev_io_start(loop, &sndwatcher);

		ev_loop(loop, 0);
	}
	FINALLY
	{}
	END;

	return EXIT_SUCCESS;
}

static void zmqcallback(struct ev_loop *loop, struct ev_io *watcher, int event)
{
	int                     zmqevent = 0;
	int                     flag = 0;
	size_t                  optlen = sizeof(zmqevent);
	struct zmqaction        *zmqact = watcher->data;

	assert(zmqact && zmqact->skt);

	flag = zmq_getsockopt(zmqact->skt, ZMQ_EVENTS, &zmqevent, &optlen);

	if (unlikely(flag == -1)) {
		x_printf(W, "occured an exception on socket : %s.", x_strerror(errno));
		ev_io_stop(loop, watcher);
		return;
	}

	if (likely((event == EV_READ) && (zmqevent & ZMQ_POLLIN))) {
		//                x_printf(D, "a new message may be received.");
		ssize_t bytes = 0;

		bytes = zmq_recv(zmqact->skt, zmqact->buff, zmqact->size, ZMQ_NOBLOCK);

		if (likely(bytes > 0)) {
			if (likely(zmqact->rcb)) {
				zmqact->rcb(zmqact->buff, bytes, zmqact->usr);
			}
		} else {
			if (unlikely(errno != EAGAIN)) {
				x_printf(W, "occured an exception on socket : %s.", x_strerror(errno));
				ev_io_stop(loop, watcher);
				return;
			} else {
				x_printf(D, "try again.");
			}
		}

		ev_feed_event(loop, watcher, event);
	} else if (likely((event == EV_WRITE) && (zmqevent & ZMQ_POLLOUT))) {
		//                x_printf(D, "a new message may be send.");
		size_t  size = 0;
		ssize_t bytes = 0;

		if (likely(zmqact->wcb)) {
			size = zmqact->size;

			zmqact->wcb(zmqact->buff, &size, zmqact->usr);

			bytes = zmq_send(zmqact->skt, zmqact->buff, size, ZMQ_NOBLOCK);

			if (unlikely(bytes < 0)) {
				if (unlikely(errno != EAGAIN)) {
					x_printf(W, "occured an exception on socket : %s.", x_strerror(errno));
					ev_io_stop(loop, watcher);
					return;
				} else {
					x_printf(D, "try again");
				}
			}
		}
	} else if (unlikely(zmqevent & ZMQ_POLLERR)) {
		x_printf(W, "occured an exception on socket.");
		ev_io_stop(loop, watcher);
		return;
	}
}

static void zmqrcb(char *buff, size_t size, void *usr)
{
	fprintf(stdout,
		PALETTE_FULL(_COLOR_RED_, _DEPTH_1_, _STYLE_NULL_, _SCENE_GREEN_)
		"-> %.*s"
		PALETTE_NULL
		"\n"
		, (int)size, buff);
}

static void zmqwcb(char *buff, size_t *size, void *usr)
{
	static int      value = 0;
	int             bytes = 0;
	size_t          ssize = *size;

	//        value = RandInt(1, 10000);

	value++;

	bytes = snprintf(buff, ssize, "%06d", value);

	fprintf(stdout,
		PALETTE_FULL(_COLOR_GREEN_, _DEPTH_1_, _STYLE_NULL_, _SCENE_RED_)
		"<- %s"
		PALETTE_NULL
		"\n"
		, buff);

	if (unlikely(bytes > (*size))) {
		x_printf(W, "not enough space.");
	} else {
		*size = bytes;
	}
}

