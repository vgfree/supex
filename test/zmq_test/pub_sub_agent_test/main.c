//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/11/9.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <stdio.h>
#include "zmq.h"
#include "libmini.h"
static const char respstr[] = "World";

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
	if (unlikely(argv != 5)) {
		x_printf(E, "Usage %s <#frontip> <#frontport> <#backip> <#backport>", argc[0]);
		return EXIT_FAILURE;
	}

	char log[32] = {};
	snprintf(log, sizeof(log), "%s.log", argc[0]);
	SLogOpen(log, NULL);

	void *volatile  ctx = NULL;
	void *volatile  front = NULL;
	void *volatile  back = NULL;
	TRY
	{
		ctx = zmq_ctx_new();
		AssertError(ctx, ENOMEM);

		front = zmq_socket(ctx, ZMQ_XSUB);
		AssertRaise(front, EXCEPT_SYS);

		back = zmq_socket(ctx, ZMQ_XPUB);
		AssertRaise(back, EXCEPT_SYS);

		int     flag = 0;
		char    addr[32] = {};

		snprintf(addr, sizeof(addr), "tcp://%s:%d", argc[1], atoi(argc[2]));
		flag = zmq_connect(front, addr);
		RAISE_SYS_ERROR(flag);

		snprintf(addr, sizeof(addr), "tcp://%s:%d", argc[3], atoi(argc[4]));
		flag = zmq_bind(back, addr);
		RAISE_SYS_ERROR(flag);

		flag = zmq_proxy(front, back, NULL);
		RAISE_SYS_ERROR(flag);
	}
	CATCH
	{}
	FINALLY
	{
		if (front) {
			zmq_close(front);
		}

		if (back) {
			zmq_close(back);
		}

		if (ctx) {
			zmq_term(ctx);
		}
	}
	END;

	return EXIT_SUCCESS;
}

