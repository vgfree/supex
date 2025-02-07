/*****************************************************************************
 * Copyright(c) shanghai... 2015-2100
 * Filename: sample.c
 * Author: shaozhenyu 18217258834@163.com
 * History:
 *        created by shaozhenyu 2015-10-12
 * Description:
 *
 ******************************************************************************/

#include <ev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "async_api.h"
#include "async_cnt.h"
#include "async_obj.h"
#include "pool_api.h"
#include "utils.h"
#include "redis_parser.h"

#define REDIS_ERR               -1
#define REDIS_OK                0

#define ASYNC_LIBEV_PEAK        2

int main()
{
	struct cnt_pool         *cpool = NULL;
	struct ev_loop          *main_loop = ev_default_loop(0);
	struct async_ctx        *ac = NULL;

	conn_xpool_init("127.0.0.1", 6000, 2000, true);

	ac = async_initial(main_loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, ASYNC_LIBEV_PEAK);

	if (ac) {
		void    *sfd = (void *)(intptr_t)-1;
		int     rc = conn_xpool_gain(&cpool, "127.0.0.1", 6000, &sfd);

		if (rc) {
			async_distory(ac);
			return -1;
		}

		char    *proto;
		int     ok = cmd_to_proto(&proto, "set key0 value0");

		if (ok == REDIS_ERR) {
			conn_xpool_push(cpool, &sfd);
			async_distory(ac);
			return -1;
		}

		async_command(ac, PROTO_TYPE_REDIS, (void *)(intptr_t)sfd, cpool, NULL, NULL, proto, strlen(proto));
		free(proto);

		ok = cmd_to_proto(&proto, "set key1 value1");

		if (ok == REDIS_ERR) {
			conn_xpool_push(cpool, &sfd);
			async_distory(ac);
			return -1;
		}

		async_command(ac, PROTO_TYPE_REDIS, (void *)(intptr_t)sfd, cpool, NULL, NULL, proto, strlen(proto));
		free(proto);

		async_startup(ac);
	}

	ev_run(main_loop, 0);
	return 0;
}

