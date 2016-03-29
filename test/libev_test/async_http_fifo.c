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

/*
 * #include "swift_api.h"
 * #include "swift_cpp_api.h"
 * #include "apply_def.h"
 */

#define ASYNC_LIBEV_PEAK        5
#define MAX_MEMBER              50
#define MAX_LEN_STRING          2048

char TEST_HTTP_FORMAT[] = "POST /%s HTTP/1.1\r\n"
	"User-Agent: curl/7.33.0\r\n"
	"Host: %s:%d\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Connection:%s\r\n"
	"Content-Length:%d\r\n"
	"Accept: */*\r\n\r\n%s";

void http_callback(struct async_ctx *ac, void *reply, void *data)
{
	printf("http_callback-----\n");

	if (reply) {
		struct net_cache *cache = reply;
		printf("reply : %s\n", cache->buf_addr);
		printf("data  : %s\n", data);
	}
}

int main()
{
	struct cnt_pool         *cpool = NULL;
	struct ev_loop          *main_loop = ev_default_loop(0);
	struct async_ctx        *ac = NULL;

	pool_api_init("www.sina.com", 80, 2000, true);

	ac = async_initial(main_loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, ASYNC_LIBEV_PEAK);

	if (ac) {
		void    *sfd = (void *)(intptr_t)-1;
		int     rc = pool_api_gain(&cpool, "www.sina.com", 80, &sfd);

		if (rc) {
			async_distory(ac);
			return -1;
		}

		/*commond 1 data*/
		char buff[MAX_LEN_STRING] = {};
		snprintf(buff, MAX_LEN_STRING - 1, TEST_HTTP_FORMAT, "", "www.sina.com", 80, "Keep-Alive", 0, "");
		/*send*/
		async_command(ac, PROTO_TYPE_HTTP, (int)(intptr_t)sfd, cpool, http_callback, NULL, buff, strlen(buff));

		rc = pool_api_gain(&cpool, "www.baidu.com", 80, &sfd);

		if (rc) {
			async_distory(ac);
			return -1;
		}

		char    buff1[MAX_LEN_STRING] = {};
		char    data[] = "async_http_fifo";
		snprintf(buff1, MAX_LEN_STRING - 1, TEST_HTTP_FORMAT, "", "www.baidu.com", 80, "Keep-Alive", 0, "");
		async_command(ac, PROTO_TYPE_HTTP, (int)(intptr_t)sfd, cpool, http_callback, data, buff1, strlen(buff1));

		async_startup(ac);
	}

	ev_run(main_loop, 0);
	return 0;
}

