//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/9/15.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <zmq.h>
#include "libmini.h"

bool checkzmqconnect(void *skt)
{
	int     opt = 0;
	int     flag = 0;
	size_t  vlen = sizeof(opt);

	assert(skt);

	x_printf(D, "test connection that whether has been completed or not.");

	flag = zmq_getsockopt(skt, ZMQ_EVENTS, &opt, &vlen);
	RAISE_SYS_ERROR(flag);

	if (opt & ZMQ_POLLIN) {
		x_printf(D, "a message may be receiver on this socket.");
		return true;
	}

	return false;
}

void testzmqrcvhwm(void *skt)
{
	int     opt = 300;
	int     flag = 0;
	size_t  vlen = sizeof(opt);

	assert(skt);

	x_printf(D, "set the hight water mark of receiver to `%d`.", opt);

	flag = zmq_setsockopt(skt, ZMQ_RCVHWM, &opt, vlen);
	RAISE_SYS_ERROR(flag);

	flag = zmq_getsockopt(skt, ZMQ_RCVHWM, &opt, &vlen);
	RAISE_SYS_ERROR(flag);

	x_printf(D, "the hight water mark of receiver is `%d`.", opt);
}

void setzmqimmediate(void *skt)
{
	int     flag = 0;
	int     opt = 1;
	size_t  vsize = sizeof(opt);

	flag = zmq_setsockopt(skt, ZMQ_IMMEDIATE, &opt, vsize);
	RAISE_SYS_ERROR(flag);
}

int main(int argc, char **argv)
{
	void    *zmqctx = NULL;
	void    *zmqskt = NULL;

	TRY
	{
		char    tcpaddr[32] = {};
		int     flag = 0;

		SLogOpen(argv[0], NULL);

		zmqctx = zmq_ctx_new();
		AssertError(zmqctx, ENOMEM);

		if (unlikely(argc != 4)) {
#if 0
			x_printf(E, "Usage %s <host> <#port> <message>.", argv[0]);
#else
			x_printf(E, "Usage %s <host> <#port> <#loops>.", argv[0]);
#endif
			RAISE(EXCEPT_ASSERT);
		}

		zmqskt = zmq_socket(zmqctx, ZMQ_PULL);
		AssertRaise(zmqskt, EXCEPT_SYS);

#if 0
		setzmqimmediate(zmqskt);
#else
		testzmqrcvhwm(zmqskt);
#endif

		snprintf(tcpaddr, sizeof(tcpaddr), "tcp://%s:%d", argv[1], atoi(argv[2]));

		x_printf(D, "starting connect to %s.", tcpaddr);

		flag = zmq_connect(zmqskt, tcpaddr);
		RAISE_SYS_ERROR(flag);

		while (!checkzmqconnect(zmqskt)) {
			sleep(1);
		}

		x_printf(D, "connect successful and start receive message.");

#if 0
		fprintf(stdout, "enter any character to continue.");
		Getch(NULL);
		fprintf(stdout, "\n");
#else
		int loops = atoi(argv[3]);
		do {
			ssize_t bytes = 0;
			char    buff[6] = {};

			bytes = zmq_recv(zmqskt, buff, sizeof(buff), 0);
			RAISE_SYS_ERROR(bytes);

			if (unlikely(bytes > sizeof(buff))) {
				x_printf(W, "receive a message that has been truncate.");
			}

			fprintf(stdout, "-> %.*s.\n", (int)(bytes > sizeof(buff) ? sizeof(buff) : bytes), buff);
		} while (--loops > 0);
#endif
	}
	CATCH
	{}
	FINALLY
	{
		int flag = 0;

		zmq_close(zmqskt);

		while (1) {
			flag = zmq_ctx_term(zmqctx);

			if (unlikely(flag < 0)) {
				if (likely(errno) == EINTR) {
					continue;
				}

				RAISE(EXCEPT_SYS);
			}

			break;
		}
	}
	END;

	return EXIT_SUCCESS;
}

