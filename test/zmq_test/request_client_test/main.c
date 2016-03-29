//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/11/5.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <stdio.h>
#include "zmq.h"
#include "libmini.h"

static const char reqstr[] = "Hello";

static void _set_zmqopt(void *skt, int opt, int value)
{
	int     flag = 0;
	size_t  vsize = sizeof(value);

	flag = zmq_setsockopt(skt, opt, &value, vsize);
	RAISE_SYS_ERROR(flag);
}

static bool _is_connect(void *skt)
{
	zmq_pollitem_t  pitem[1] = { { .fd = -1 } };
	int             rc = 0;

	pitem[0].events = ZMQ_POLLOUT;
	pitem[0].socket = skt;

	rc = zmq_poll(pitem, 1, 3000);
	AssertRaise(rc > -1, EXCEPT_SYS);

	/* enable to send*/
	if ((rc > 0) && (pitem[0].revents & ZMQ_POLLOUT)) {
		return true;
	}

	return false;
}

int main(int argv, char **argc)
{
	if (unlikely(argv != 4)) {
		x_printf(E, "Usage %s <ip> <#port> <#loops>", argc[0]);
		return EXIT_FAILURE;
	}

	char log[32] = {};
	snprintf(log, sizeof(log), "%s.log", argc[0]);
	SLogOpen(log, NULL);

	void *volatile  ctx = NULL;
	void *volatile  skt = NULL;
	TRY
	{
		ctx = zmq_ctx_new();
		AssertRaise(ctx, EXCEPT_SYS);

		skt = zmq_socket(ctx, ZMQ_REQ);
		AssertRaise(skt, EXCEPT_SYS);

		_set_zmqopt(skt, ZMQ_IMMEDIATE, 1);

		int     rc = 0;
		char    addr[64] = {};

		snprintf(addr, sizeof(addr), "tcp://%s:%d", argc[1], atoi(argc[2]));
		rc = zmq_connect(skt, addr);
		RAISE_SYS_ERROR(rc);

		bool iscnt = false;
		iscnt = _is_connect(skt);
		AssertError(iscnt, ETIMEDOUT);

		int     i = 0;
		int     cnt = atoi(argc[3]);
		char    buff[1024] = {};

		GetProcessID();
		GetThreadID();

		do {
			/*send request*/
			int bytes = 0;
			bytes = snprintf(buff, sizeof(buff), "%ld : %s", g_ThreadID, reqstr);

			bytes = zmq_send(skt, buff, bytes, 0);
			RAISE_SYS_ERROR(bytes);

			x_printf(I, "send %s --> ", buff);

			/*recv response*/
			bytes = zmq_recv(skt, buff, sizeof(buff), 0);
			RAISE_SYS_ERROR(bytes);

			if (unlikely(bytes > sizeof(buff))) {
				x_printf(I, "--> recv %.*s", (int)sizeof(buff), buff);
			} else {
				x_printf(I, "--> recv %.*s", bytes, buff);
			}
		} while (++i < cnt);
	}
	CATCH
	{}
	FINALLY
	{
		if (ctx) {
			if (skt) {
				zmq_close(skt);
			}

			zmq_ctx_term(ctx);
		}
	}
	END;

	return EXIT_SUCCESS;
}

