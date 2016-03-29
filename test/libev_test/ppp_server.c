#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ev.h>

#include "async_api.h"
#include "async_cnt.h"
#include "async_obj.h"
#include "pool_api.h"
#include "utils.h"
#include "redis_parser.h"
#include "cJSON.h"

#define DEFAULT_PORT            10003
#define MAXLINE                 4096

#define X_DONE_OK               0
#define X_IO_ERROR              -1
#define X_DATA_TOO_LARGE        -2
#define X_MALLOC_FAILED         -3
#define X_PARSE_ERROR           -4
#define X_INTERIOR_ERROR        -5
#define X_REQUEST_ERROR         -6
#define X_EXECUTE_ERROR         -7
#define X_REQUEST_QUIT          -8
#define X_KV_TOO_MUCH           -9
#define X_NAME_TOO_LONG         -10

/*
 *
 *   int send_data_to_PPP(char * data)
 *   {
 *        //初始化async libev
 *        struct cnt_pool *cpool = NULL;
 *        struct ev_loop *main_loop = ev_default_loop(0);
 *        struct async_ctx *ac = NULL;
 *
 *        pool_api_init("127.0.0.1", 30001, 2000, true);
 *
 *        ac = async_initial(main_loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, 0);
 *        if (ac) {
 *                void *sfd = (void*)(intptr_t)-1;
 *                int rc = pool_api_gain( &cpool, "127.0.0.1", 30001, &sfd);
 *                if (rc){
 *                        async_distory(ac);
 *                        return -1;
 *                }
 *
 *                async_command(ac, PROTO_TYPE_HTTP, sfd, cpool, NULL, NULL, data, strlen(data));
 *
 *                async_startup(ac);
 *        }
 *        ev_run(main_loop, 0);
 *        return 0;
 *   }
 *
 */
void *joint_cjson(char *data, int connect_fd)
{
	char *json, *out;

	json = strstr(data, "{");

	if (NULL == json) {
		return NULL;
	}

	cJSON *new_json = cJSON_Parse(json);
	cJSON_AddStringToObject(new_json, "ADD", "KKKKKKKKKKKKK");

	out = cJSON_Print(new_json);

	char buff[2048] = {};
	strncpy(buff, data, strlen(data) - strlen(json));
	strncat(buff, out, strlen(out));

	printf("%s\n", buff);

	if (!fork()) {
		if (send(connect_fd, buff, strlen(buff), 0) == -1) {
			perror("send error");
		}

		close(connect_fd);
		exit(0);
	}

	// send_data_to_PPP(buff);
}

void *do_recv_data(struct net_cache *cache, int sfd, struct http_parse_info *parseinfo)
{
	int     ret = 0;
	int     status = 0;

	ret = net_recv(cache, sfd, &status);

	printf("net_recv : %s\n", cache->buf_addr);

	if (ret > 0) {
		do {
			/*no more memoryi or data too large*/
			if ((status == X_MALLOC_FAILED) || (status == X_DATA_TOO_LARGE)) {
				return NULL;
			}

			/* parse recive data */
			if (!http_parse_response(parseinfo)) {
				/*data not all,go on recive*/
				return NULL;
			}

			/*over*/
			if (parseinfo->hs.err != HPE_OK) {
				status = X_PARSE_ERROR;
				return NULL;
			}
		} while (0);
	}
}

int main(int argc, char **argv)
{
	int                     socket_fd, connect_fd;
	struct sockaddr_in      servaddr;
	char                    buff[2048];
	int                     n;

	// 初始化cache
	struct net_cache *cache;

	cache = (struct net_cache *)malloc(sizeof(struct net_cache));

	if (!cache) {
		return -1;
	}

	cache_init(cache);

	// 初始化Socket
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}

	// 初始化
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	// IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
	servaddr.sin_port = htons(DEFAULT_PORT);	// 设置的端口为DEFAULT_PORT

	// 将本地地址绑定到所创建的套接字上
	if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}

	// 开始监听是否有客户端连接
	if (listen(socket_fd, 10) == -1) {
		printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}

	while (1) {
		// 阻塞直到有客户端连接，不然多浪费CPU资源。
		if ((connect_fd = accept(socket_fd, (struct sockaddr *)NULL, NULL)) == -1) {
			printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
			continue;
		}

		// 初始化http_parse_info
		struct http_parse_info *parseinfo;
		parseinfo = (struct http_parse_info *)malloc(sizeof(struct http_parse_info));

		if (!parseinfo) {
			return -1;
		}

		http_parse_init(parseinfo, &cache->buf_addr, &cache->get_size);

		// 接受客户端传过来的数据
		do_recv_data(cache, connect_fd, parseinfo);

		close(connect_fd);
	}

	close(socket_fd);
}

