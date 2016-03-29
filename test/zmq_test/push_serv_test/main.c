//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/9/15.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <zmq.h>
#include "libmini.h"

void testzmqsndhwm(void *skt)
{
	int     opt = 300;
	int     flag = 0;
	size_t  vlen = sizeof(opt);

	assert(skt);

	x_printf(D, "set the hight water mark of sender to `%d`.", opt);

	flag = zmq_setsockopt(skt, ZMQ_SNDHWM, &opt, vlen);
	RAISE_SYS_ERROR(flag);

	flag = zmq_getsockopt(skt, ZMQ_SNDHWM, &opt, &vlen);
	RAISE_SYS_ERROR(flag);

	x_printf(D, "the hight water mark of sender is `%d`.", opt);
}

int main(int argc, char **argv)
{
	void    *zmqctx = NULL;
	void    *zmqskt = NULL;

	TRY
	{
		char    tcpaddr[32] = {};
		int     flag = 0;

		zmqctx = zmq_ctx_new();
		AssertError(zmqctx, ENOMEM);

		if (unlikely(argc != 3)) {
			x_printf(E, "Usage %s <host> <#port>.", argv[0]);
			RAISE(EXCEPT_ASSERT);
		}

		zmqskt = zmq_socket(zmqctx, ZMQ_PUSH);
		AssertRaise(zmqskt, EXCEPT_SYS);

		testzmqsndhwm(zmqskt);

		snprintf(tcpaddr, sizeof(tcpaddr), "tcp://%s:%d", argv[1], atoi(argv[2]));

		x_printf(D, "starting bind to %s.", tcpaddr);

		flag = zmq_bind(zmqskt, tcpaddr);
		RAISE_SYS_ERROR(flag);

		x_printf(D, "bind successful and start send message.");

#if 0
		fprintf(stdout, "enter any character to continue.");
		Getch(NULL);
		fprintf(stdout, "\n");
#endif

		int seq = 0;

		while (1) {
#if 1
			char    buff[6] = {};
			ssize_t bytes = 0;

			snprintf(buff, sizeof(buff), "%d", seq++);

			bytes = zmq_send(zmqskt, buff, sizeof(buff), 0);
			RAISE_SYS_ERROR(bytes);

			fprintf(stdout, "-> %s\n", buff);
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

