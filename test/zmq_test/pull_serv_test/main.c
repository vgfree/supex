//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/9/15.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <zmq.h>
#include "libmini.h"

void testzmqrcvhwm(void *skt)
{
	int     opt = 300;
	int     flag = 0;
	int     vlen = sizeof(opt);

	assert(skt);

	x_printf(D, "set the hight water mark of receiver to `%d`.", opt);

	flag = zmq_setsockopt(skt, ZMQ_RCVHWM, &opt, vlen);
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

		if (unlikely(argc != 3)) {
			x_printf(E, "Usage %s <host> <#port>.", argv[0]);
			RAISE(EXCEPT_ASSERT);
		}

		zmqskt = zmq_socket(zmqctx, ZMQ_PULL);
		AssertRaise(zmqskt, EXCEPT_SYS);

		testzmqrcvhwm(zmqskt);

		snprintf(tcpaddr, sizeof(tcpaddr), "tcp://%s:%d", argv[1], atoi(argv[2]));

		x_printf(D, "starting bind to %s.", tcpaddr);

		flag = zmq_bind(zmqskt, tcpaddr);
		RAISE_SYS_ERROR(flag);

		x_printf(D, "bind successful and start receive message.");

#if 0
		fprintf(stdout, "enter any character to continue.");
		Getch(NULL);
		fprintf(stdout, "\n");
#endif

		while (1) {
#if 1
			char    buff[6] = {};
			ssize_t bytes = 0;

			bytes = zmq_recv(zmqskt, buff, sizeof(buff), 0);
			RAISE_SYS_ERROR(bytes);

			if (unlikely(bytes > sizeof(buff))) {
				x_printf(W, "message was truncate.");
			}

			fprintf(stdout, "-> %.*s\n", (int)(bytes > sizeof(buff) ? sizeof(buff) : bytes), buff);
#else
			sleep(1);
#endif
		}
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

