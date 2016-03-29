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

static void _set_zmqopt(void *skt, int opt, int value)
{
	int     flag = 0;
	size_t  vsize = sizeof(value);

	flag = zmq_setsockopt(skt, opt, &value, vsize);
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
	SLogOpen(log, SLOG_W_LEVEL);

	void *volatile  ctx = NULL;
	void *volatile  skt = NULL;
	TRY
	{
		ctx = zmq_ctx_new();
		AssertRaise(ctx, EXCEPT_SYS);

		skt = zmq_socket(ctx, ZMQ_PUB);
		AssertRaise(skt, EXCEPT_SYS);

		//                _set_zmqopt(skt, ZMQ_SNDHWM, 10);

		int     rc = 0;
		char    addr[64] = {};
		snprintf(addr, sizeof(addr), "tcp://%s:%d", argc[1], atoi(argc[2]));
		rc = zmq_bind(skt, addr);
		RAISE_SYS_ERROR(rc);

		rc = zmq_bind(skt, "ipc://rand.ipc");
		RAISE_SYS_ERROR(rc);

		Rand();

		char buff[10] = {};

		while (1) {
			int value;

			int bytes = 0;

			value = RandInt(190, 200);
			bytes = snprintf(buff, sizeof(buff), "%d", value);
			bytes = zmq_send(skt, buff, bytes, ZMQ_SNDMORE);
			RAISE_SYS_ERROR(bytes);

			x_printf(I, "field 1 : %d", value);

			value = RandInt(1000, 2000);
			bytes = snprintf(buff, sizeof(buff), "%d", value);
			bytes = zmq_send(skt, buff, bytes, ZMQ_SNDMORE);
			RAISE_SYS_ERROR(bytes);

			x_printf(I, "field 2 : %d", value);

			value = RandInt(100000, 200000);
			bytes = snprintf(buff, sizeof(buff), "%d", value);
			bytes = zmq_send(skt, buff, bytes, 0);
			RAISE_SYS_ERROR(bytes);

			x_printf(I, "field 3 : %d", value);
		}
	}
	CATCH
	{}
	FINALLY
	{}
	END;

	return EXIT_SUCCESS;
}

