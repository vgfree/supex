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

void *front_to_back(void *item)
{
	int             more = 0;
	size_t          len = 0;
	int             bytes = 0;
	int             rc = 0;
	char            buff[4096] = {};
	zmq_pollitem_t  *pitem = item;

	while (1) {
		while (1) {
			bytes = zmq_recv(pitem[0].socket, buff, sizeof(buff), 0);
			RAISE_SYS_ERROR(bytes);

			len = sizeof(more);
			rc = zmq_getsockopt(pitem[0].socket, ZMQ_RCVMORE, &more, &len);
			RAISE_SYS_ERROR(rc);

			bytes = zmq_send(pitem[1].socket, buff,
					MIN(bytes, sizeof(buff)),
					more == 0 ? 0 : ZMQ_SNDMORE);
			RAISE_SYS_ERROR(bytes);

			if (more == 0) {
				break;
			}
		}
	}

	return NULL;
}

void *back_to_front(void *item)
{
	int             more = 0;
	size_t          len = 0;
	int             bytes = 0;
	int             rc = 0;
	char            buff[4096] = {};
	zmq_pollitem_t  *pitem = item;

	while (1) {
		while (1) {
			bytes = zmq_recv(pitem[1].socket, buff, sizeof(buff), 0);
			RAISE_SYS_ERROR(bytes);

			len = sizeof(more);
			rc = zmq_getsockopt(pitem[1].socket, ZMQ_RCVMORE, &more, &len);
			RAISE_SYS_ERROR(rc);

			bytes = zmq_send(pitem[0].socket, buff,
					MIN(bytes, sizeof(buff)),
					more == 0 ? 0 : ZMQ_SNDMORE);
			RAISE_SYS_ERROR(bytes);

			if (more == 0) {
				break;
			}
		}
	}

	return NULL;
}

int main(int argv, char **argc)
{
	if (unlikely(argv != 3)) {
		x_printf(E, "Usage %s <#frontport> <#backport>", argc[0]);
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
		AssertRaise(ctx, EXCEPT_SYS);

		front = zmq_socket(ctx, ZMQ_ROUTER);	/*此端连接，数据被公平排队接收？*/
		AssertRaise(front, EXCEPT_SYS);

		back = zmq_socket(ctx, ZMQ_DEALER);	/*此端连接，数据被平均分配发送？*/
		AssertRaise(back, EXCEPT_SYS);

		int     flag = 0;
		char    addr[32] = {};

		snprintf(addr, sizeof(addr), "tcp://*:%d", atoi(argc[1]));
		flag = zmq_bind(front, addr);
		RAISE_SYS_ERROR(flag);

		snprintf(addr, sizeof(addr), "tcp://*:%d", atoi(argc[2]));
		flag = zmq_bind(back, addr);
		RAISE_SYS_ERROR(flag);
#if 0
		zmq_pollitem_t pitem[] = {
			{ .socket = front, .fd = -1, .events = ZMQ_POLLIN },
			{ .socket = back,  .fd = -1, .events = ZMQ_POLLIN },
		};

  #if 0
		/*不能多线程使用同一个套接字*/
		pthread_t ptid[2];

		pthread_create(&ptid[0], NULL, front_to_back, pitem);
		pthread_create(&ptid[1], NULL, back_to_front, pitem);

		int i = 0;

		for (i = 0; i < DIM(ptid); i++) {
			pthread_join(ptid[i], NULL);
		}

  #else
		char buff[4096] = {};

		while (1) {
			int     more = 0;
			size_t  len = 0;
			int     bytes = 0;
			int     n = 0;
			int     rc = 0;

			n = zmq_poll(pitem, DIM(pitem), -1);
			RAISE_SYS_ERROR(n);

			if (pitem[0].revents & ZMQ_POLLIN) {
				while (1) {
					bytes = zmq_recv(front, buff, sizeof(buff), 0);
					RAISE_SYS_ERROR(bytes);

					len = sizeof(more);
					rc = zmq_getsockopt(front, ZMQ_RCVMORE, &more, &len);
					RAISE_SYS_ERROR(rc);

					bytes = zmq_send(back, buff,
							MIN(bytes, sizeof(buff)),
							more == 0 ? 0 : ZMQ_SNDMORE);
					RAISE_SYS_ERROR(bytes);

					if (more == 0) {
						break;
					}
				}
			}

			if (pitem[1].revents & ZMQ_POLLIN) {
				while (1) {
					bytes = zmq_recv(back, buff, sizeof(buff), 0);
					RAISE_SYS_ERROR(bytes);

					len = sizeof(more);
					rc = zmq_getsockopt(back, ZMQ_RCVMORE, &more, &len);
					RAISE_SYS_ERROR(rc);

					bytes = zmq_send(front, buff,
							MIN(bytes, sizeof(buff)),
							more == 0 ? 0 : ZMQ_SNDMORE);
					RAISE_SYS_ERROR(bytes);

					if (more == 0) {
						break;
					}
				}
			}
		}
  #endif	/* if 0 */
#else
		flag = zmq_proxy(front, back, NULL);
		RAISE_SYS_ERROR(flag);
#endif		/* if 0 */
	}
	CATCH
	{}
	FINALLY
	{}
	END;

	return EXIT_SUCCESS;
}

