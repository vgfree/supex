//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/11/7.
//  Copyright © 2015年 zk. All rights reserved.
//
#include <stdio.h>
#include "zmq.h"
#include "libmini.h"
static void _set_zmqopt_int(void *skt, int opt, int value)
{
	int     flag = 0;
	size_t  vsize = sizeof(value);

	flag = zmq_setsockopt(skt, opt, &value, vsize);
	RAISE_SYS_ERROR(flag);
}

static void _set_zmqopt_string(void *skt, int opt, const char *value)
{
	int     flag = 0;
	size_t  vsize = strlen(value);

	assert(value && skt);
	flag = zmq_setsockopt(skt, opt, value, vsize);
	RAISE_SYS_ERROR(flag);
}

int main(int argv, char **argc)
{
	if (unlikely(argv != 3)) {
		x_printf(E, "Usage %s <ip> <#port>", argc[0]);
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

		skt = zmq_socket(ctx, ZMQ_SUB);
		AssertRaise(skt, EXCEPT_SYS);

		int     rc = 0;
		char    addr[64] = {};
		snprintf(addr, sizeof(addr), "tcp://%s:%d", argc[1], atoi(argc[2]));
		rc = zmq_connect(skt, addr);
		RAISE_SYS_ERROR(rc);
		rc = zmq_connect(skt, "ipc://rand.ipc");
		RAISE_SYS_ERROR(rc);

		_set_zmqopt_string(skt, ZMQ_SUBSCRIBE, "195");

		char buff[10] = {};

		while (1) {
			int     bytes = 0;
			int     rc = 0;
			int     more = 0;
			int     i = 0;
			size_t  size = 0;

			while (1) {
				size = sizeof(more);

				bytes = zmq_recv(skt, buff, sizeof(buff), 0);
				RAISE_SYS_ERROR(bytes);

				x_printf(I, "field %d : %.*s", ++i, bytes, buff);

				rc = zmq_getsockopt(skt, ZMQ_RCVMORE, &more, &size);
				RAISE_SYS_ERROR(rc);

				if (!more) {
					break;
				}
			}
		}
	}
	CATCH
	{}
	FINALLY
	{}
	END;

	return EXIT_SUCCESS;
}

