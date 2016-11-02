/* 文件: zmqskt.c
 *   版权:
 *   描述: 本文件提供zeroMQ
 *   历史: 2015/3/12 新文件建立by l00167671/luokaihui
 */
#include <stdio.h>
#include <assert.h>
#include <zmq.h>
#include <unistd.h>
#include "zmqskt.h"
#include "mfptp_api.h"

extern struct mfptp_settings g_mfptp_settings;

/* 名  称: init_push_socket
 * 功  能: push -pull 模型中的push端SOCKET建立
 * 参  数:
 * 返回值: 非NULL 表示成功，NULL 表示失败
 * 修  改: 新生成函数l00167671 at 2015/2/28
 */
void *mfptp_init_push_socket(char *end_point)
{
	char    connect_point_front[64];
	void    *context = zmq_ctx_new();
	void    *push_skt = zmq_socket(context, ZMQ_PUSH);
	int     r = 1;

	sprintf(connect_point_front, "tcp://127.0.0.1:%d", g_mfptp_settings.conf->file_info.front_port);

	if (NULL != push_skt) {
		r = zmq_connect(push_skt, connect_point_front);

		if (r == -1) {
			zmq_print_error();
			zmq_close(push_skt);
			return NULL;
		}
	} else {
		return NULL;
	}

	return push_skt;
}

/* 名  称: mfptp_zmq_push_frame
 * 功  能: push -pull 模型中发送一个frame 给pull 端
 * 参  数:
 * 返回值: TRUE 表示成功，FALSE 表示失败
 * 修  改: 新生成函数l00167671 at 2015/2/28
 */
int mfptp_zmq_push_frame1(void *skt, char *buf, int len)
{
	// printf("publish message: %s\n", buf);
	if (len <= 0) {
		return 0;
	}

	if (buf == 0) {
		return 0;
	}

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, len + 1);
	memcpy(zmq_msg_data(&msg), buf, len + 1);
	zmq_msg_send(&msg, skt, 0);
	zmq_msg_close(&msg);

	usleep(rand() % 100 * 1000);
}

int mfptp_zmq_push_frame(void *skt, char *buf, int len, int more)
{
	int r;

	if ((NULL == buf) || (len < 0)) {
		return FALSE;
	}

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, len);
	memcpy(zmq_msg_data(&msg), buf, len);
	r = zmq_msg_send(&msg, skt, more);
	zmq_msg_close(&msg);

	if (r == -1) {
		zmq_print_error();
		LOG(LOG_NET_UPLINK, D, "mfptp_zmq_push_frame-------send failed---- %d\n", r);
		printf("mfptp_zmq_push_frame-------发送失败---- %d\n", r);
		return FALSE;
	} else {
		LOG(LOG_NET_UPLINK, D, "mfptp_zmq_push_frame------send successful---- %d\n", r);
		printf("mfptp_zmq_push_frame-------发送成功---- %d\n", r);
	}

	return TRUE;
}

/* 名  称: start_proxy
 * 功  能: push -pull 模型的PROXY
 * 参  数:
 * 返回值: TRUE 表示成功，FALSE 表示失败
 * 修  改: 新生成函数l00167671 at 2015/2/28
 */
int start_proxy(void *arg)
{
	broker_para     *bpra = (broker_para *)arg;
	const char      *front_addr = bpra->front_addr;
	const char      *back_addr = bpra->back_addr;
	char            buf[512];
	char            connect_point[64];
	void            *context = zmq_ctx_new();

	assert(context && front_addr && back_addr);
	void    *frontend = zmq_socket(context, ZMQ_PULL);
	void    *backend = zmq_socket(context, ZMQ_PUSH);

	assert(frontend && backend);
	sprintf(connect_point, "tcp://*:%d", g_mfptp_settings.conf->file_info.front_port);
	int r = zmq_bind(frontend, connect_point);
	printf("前端绑定地址:%s\n", connect_point);
	assert(r == 0);
	sprintf(connect_point, "tcp://*:%d", g_mfptp_settings.conf->file_info.back_port);
	r = zmq_bind(backend, connect_point);
	printf("后端绑定地址:%s\n", connect_point);
	assert(r == 0);
	printf("broker started...\n");
	zmq_proxy(frontend, backend, NULL);

#if 0
	zmq_pollitem_t items[] =
	{
		{ frontend, 0, ZMQ_POLLIN, 0 },
		{ backend,  0, ZMQ_POLLIN, 0 },
	};

	while (1) {
		zmq_msg_t message;
		printf("ahhhhhhhhhhhhhhhhhhhhhh\n");
		r = zmq_poll(items, 2, -1);

		if (r == -1) {
			zmq_print_error();
			break;
		}

		printf("a-----------------------------------------\n");

		if (items[0].revents & ZMQ_POLLIN) {
			while (1) {
				zmq_msg_init(&message);
				r = zmq_msg_recv(&message, frontend, 0);

				if (r == -1) {
					zmq_print_error();
					break;
				} else {
					int len = zmq_msg_size(&message);
					// char *s=(char *)zmq_msg_data(&message);
					// memcpy(buf,s,len);
					// buf[len]=0;
					// printf("转发haha %s\n",buf);
					printf("zm got %d\n", len);
				}

				int more = zmq_msg_more(&message);
				r = zmq_msg_send(&message, backend, more ? ZMQ_SNDMORE : 0);

				if (r == -1) {
					zmq_print_error();
					zmq_msg_close(&message);
					break;
				}

				zmq_msg_close(&message);

				if (!more) {
					break;
				}
			}
		}

		if (items[1].revents & ZMQ_POLLIN) {
			while (1) {
				zmq_msg_init(&message);
				r = zmq_msg_recv(&message, backend, 0);

				if (r == -1) {
					zmq_print_error();
					zmq_msg_close(&message);
					break;
				} else {
					printf("got %d\n", r);
				}

				int more = zmq_msg_more(&message);
				r = zmq_msg_send(&message, frontend, more ? ZMQ_SNDMORE : 0);

				if (r == -1) {
					zmq_print_error();
					zmq_msg_close(&message);
					break;
				} else {
					printf("got from %s send %d to %s more=%d\n", back_addr, r, front_addr, more);
				}

				zmq_msg_close(&message);

				if (!more) {
					break;
				}
			}
		}
	}
#endif	/* if 0 */
	printf("broker closed...\n");
	return 1;
}

