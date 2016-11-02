/* 文件: zmq_weibo_forward.c
 *   版权:
 *   描述: 本文件主要提供将业务服务器发送过来的zmq协议报文转为数据并放到用户缓冲区中
 *   历史: 2015/3/12 新文件建立by l00167671/luokaihui
 */

#include "zmq_weibo_forward.h"
#include "mfptp_users_rbtree.h"
#include "net_cache.h"
#include "mfptp_parser.h"
#include "mfptp_api.h"
#include "mfptp_task.h"
#include <time.h>

extern int                      G_WORKER_COUNTS;
extern MFPTP_WORKER_PTHREAD     *g_worker_pthread;
extern struct mfptp_settings    g_mfptp_settings;
#define zmq_print_error()			       \
	printf("%s[%d]: %d: %s\n", __FILE__, __LINE__, \
		zmq_errno(), zmq_strerror(zmq_errno()));

extern rb_root user_tree;

/* 名  称: mfptp_usr_weibo_forward
 * 功  能: 从业务服务器拉weibo消息，放到对应用户的缓冲区中
 * 参  数:
 * 返回值: TRUE 表示成功，FALSE 表示失败
 * 修  改: 新生成函数l00167671 at 2015/3/23
 */
void *mfptp_usr_weibo_forward(void *arg)
{
	int                     major, minor, patch;
	struct mfptp_task_node  node;
	int                     more;
	int                     len;
	char                    name_buf[1024];
	char                    connect_point[64];
	char                    *msg_buf;
	char                    *ptr;
	MFPTP_WORKER_PTHREAD    *p_worker = NULL;
	int                     mfptp_len;

#define MAX_BUF_LEN 1024 * 1024 * 6
	char *data = (char *)malloc(MAX_BUF_LEN);

	if (data == NULL) {
		fprintf(stderr, "分配内存出错\n");
		LOG(LOG_INIT, D, "malloc error!\n");
		exit(0);
	}

	ptr = data;
	zmq_version(&major, &minor, &patch);
	int version = ZMQ_MAKE_VERSION(major, minor, patch);

	if (1) {
		printf("zmq library expect version %d but get %d.\n", ZMQ_VERSION, version);
		LOG(LOG_INIT, D, "zmq library expect version %d but get %d.\n", ZMQ_VERSION, version);
	}

	void    *context = zmq_ctx_new();
	void    *subscriber = zmq_socket(context, ZMQ_PULL);
	// int" r = zmq_connect(subscriber, "tcp://192.168.11.66:19991") ;
	sprintf(connect_point, "tcp://*:%d", g_mfptp_settings.conf->file_info.usr_msg_port);
	int r = zmq_bind(subscriber, connect_point);
	assert(r == 0);

	while (1) {
		int             count = 0;
		zmq_msg_t       name;
		zmq_msg_t       msg;
		mfptp_len = 0;
		ptr = data;

		/* 接收到的第一帧数据是用户标识符 */
		zmq_msg_init(&name);
		zmq_msg_recv(&name, subscriber, 0);
		count = count + 1;
		more = zmq_msg_more(&name);
		int name_len = zmq_msg_size(&name);
		printf("zmq_msg %d\n", name_len);
		memcpy(name_buf, (char *)zmq_msg_data(&name), name_len);
		name_buf[name_len] = 0;
		LOG(LOG_NET_DOWNLINK, D, "receive weibo send to :%s, more=%d\n", name_buf, more);
		zmq_msg_close(&name);

		unsigned int    hash = mfptp_bkdr_hash(name_buf);
		int             idx = hash % G_WORKER_COUNTS;
		idx = 0;
		p_worker = &g_worker_pthread[idx];

		if (1) {
			len = mfptp_pack_hdr(ptr, 0x1, PUSH_METHOD, 1);
			ptr = ptr + len;
			mfptp_len += len;

			while (more) {
				zmq_msg_init(&msg);
				zmq_msg_recv(&msg, subscriber, 0);
				count = count + 1;
				more = zmq_msg_more(&msg);
				len = zmq_msg_size(&msg);
				LOG(LOG_NET_DOWNLINK, D, "zmq_msg %d\n", len);

				if (len > 0) {
					msg_buf = (char *)zmq_msg_data(&msg);
					mfptp_log(name_buf, msg_buf, len, 0);
					len = mfptp_pack_frame(msg_buf, len, ptr, more);
					ptr += len;
					mfptp_len += len;
				}

				zmq_msg_close(&msg);
			}

			LOG(LOG_NET_DOWNLINK, M, "push weibo to %s,len %d\n\n", name_buf, mfptp_len);
			LOG(LOG_NET_DOWNLINK, D, "-----count = %d\n", count);

			if ((name_len == 16) && (count == 2)) {
				memcpy(node.usrID, name_buf + 1, name_len);
				node.data = malloc(mfptp_len);
				memcpy(node.data, data, mfptp_len);
				node.data_size = mfptp_len;

				if ('0' == *name_buf) {
					node.mode = enMODE_USR;
				} else {
					node.mode = enMODE_GRP;
				}

				time(&node.stamp);
				LOG(LOG_NET_DOWNLINK, M, "push user weibo to supex list!\n\n");
				free_queue_push(&(p_worker->tlist), (void *)&node);
				supex_evuv_wake(&(p_worker->evuv));
			} else {
				continue;
			}
		} else {
			printf("user is not in server %s\n", name_buf);
		}
	}

	zmq_close(subscriber);
	zmq_ctx_destroy(context);
	return NULL;
}

/*
 * 函数名:mfptp_edit_user_channel_info
 * 功能:编译用户和频道信息
 * 参数:oper　操作类型，'1' 是用户加入某个频道，'2'是用户脱离某个频道, imei用户的imei, channel 用户的频道
 * 返回值:目前只返回TRUE
 */
int mfptp_edit_user_channel_info(unsigned char oper, char *imei, char *channel)
{
	if ((NULL == imei) || (NULL == channel)) {
		if (USER_ADD_CHANNEL == oper) {
			topo_add_user_record(channel, imei);
		} else if (USER_DELETE_CHANNEL == oper) {
			topo_delete_user_record(channel, imei);
		} else {}
	}

	return TRUE;
}

/* 名  称: mfptp_gp_weibo_forward
 * 功  能: 取业务服务器的组weibo,并放到每个用户的写缓冲区中
 * 参  数:
 * 返回值: TRUE 表示成功，FALSE 表示失败
 * 修  改: 新生成函数l00167671 at 2015/3/23
 */
void *mfptp_gp_weibo_forward(void *arg)
{
	int                     major, minor, patch;
	struct user_info        *usr;
	struct mfptp_task_node  node;
	int                     more;
	int                     len;
	char                    name_buf[1024];
	char                    connect_point[64];
	char                    *msg_buf;
	char                    *ptr;
	MFPTP_WORKER_PTHREAD    *p_worker = NULL;
	int                     mfptp_len;

#define MAX_BUF_LEN 1024 * 1024 * 6
	char *data = (char *)malloc(MAX_BUF_LEN);

	if (data == NULL) {
		fprintf(stderr, "分配内存出错\n");
		LOG(LOG_INIT, D, " gp weibo ,malloc error!\n\n");
		exit(0);
	}

	ptr = data;

	/* 获取ZMQ版本号 */
	zmq_version(&major, &minor, &patch);
	int version = ZMQ_MAKE_VERSION(major, minor, patch);

	void    *context = zmq_ctx_new();
	void    *subscriber = zmq_socket(context, ZMQ_SUB);
	sprintf(connect_point, "tcp://*:%d", g_mfptp_settings.conf->file_info.gp_msg_port);
	const char *filter = "";
	assert(zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, strlen(filter)) == 0);
	int r = zmq_bind(subscriber, connect_point);
	assert(r == 0);

	// 订阅模式,订阅用户的的修改信息
	void    *user_info_subscriber = zmq_socket(context, ZMQ_SUB);
	char    connect_address[64] = { 0 };
	sprintf(connect_address, "tcp://*:%d", g_mfptp_settings.conf->file_info.edit_user_info_port);
	// 订阅所有消息
	assert(zmq_setsockopt(user_info_subscriber, ZMQ_SUBSCRIBE, filter, strlen(filter)) == 0);
	r = zmq_bind(user_info_subscriber, connect_address);
	assert(0 == r);
	zmq_pollitem_t items[] = {
		{ subscriber,           0, ZMQ_POLLIN, 0 },
		{ user_info_subscriber, 0, ZMQ_POLLIN, 0 }
	};

	char    oper = USER_ILLEGAL_OPER;			// 操作类型
	char    imei[IMEI_LEN + 1] = { 0 };			// IMEI
	char    channel[MAX_CHANNEL_NAME_LEN] = { 0 };		// channel

	enum RECV_TYPE { OPER = 0, IMEI, CHANNEL };		// 消息中一共有三个字段，操作类型，ＩＭＥＩ,频道
	enum RECV_TYPE current_type = OPER;			// 用户当前的读取数据字段类型

	while (1) {
		zmq_poll(items, 2, -1);
		LOG(LOG_NET_DOWNLINK, M, "down link module start, push data to group！\n");

		if (items[0].revents & ZMQ_POLLIN) {
			zmq_msg_t       name;
			zmq_msg_t       msg;
			mfptp_len = 0;
			ptr = data;

			/* 接收到的第一帧数据是用户标识符 */
			zmq_msg_init(&name);
			zmq_msg_recv(&name, subscriber, 0);
			more = zmq_msg_more(&name);
			int name_len = zmq_msg_size(&name);
			memcpy(name_buf, (char *)zmq_msg_data(&name), name_len);
			name_buf[name_len] = 0;
			zmq_msg_close(&name);

			/* 对用户标识符(IMEI)做哈希 */
			unsigned int    hash = mfptp_bkdr_hash(name_buf);
			int             idx = hash % G_WORKER_COUNTS;
			idx = 0;
			p_worker = &g_worker_pthread[idx];

			if (1) {
				len = mfptp_pack_hdr(ptr, 0x1, PUSH_METHOD, 1);
				ptr = ptr + len;
				mfptp_len += len;

				while (more) {
					zmq_msg_init(&msg);
					zmq_msg_recv(&msg, subscriber, 0);
					more = zmq_msg_more(&msg);
					len = zmq_msg_size(&msg);

					if (len > 0) {
						msg_buf = (char *)zmq_msg_data(&msg);
						mfptp_log(name_buf, msg_buf, len, 0);
						len = mfptp_pack_frame(msg_buf, len, ptr, more);
						ptr += len;
						mfptp_len += len;
					}

					zmq_msg_close(&msg);
				}

				printf("将微博推送给%s,长度是%d\n\n", name_buf, mfptp_len);
				LOG(LOG_NET_DOWNLINK, D, "push weibo data to %s, len %d\n\n", name_buf, mfptp_len);
				memcpy(node.usrID, name_buf + 1, strlen(name_buf));
				node.data = malloc(mfptp_len);
				memcpy(node.data, data, mfptp_len);
				node.data_size = mfptp_len;

				if ('0' == *name_buf) {
					node.mode = enMODE_USR;
				} else {
					node.mode = enMODE_GRP;
				}

				time(&node.stamp);
				LOG(LOG_NET_DOWNLINK, D, "now,push downlink data to supex list \n ");
				free_queue_push(&(p_worker->tlist), (void *)&node);
				supex_evuv_wake(&(p_worker->evuv));
			} else {
				printf("user %s is not in server !\n", name_buf);
				LOG(LOG_NET_DOWNLINK, D, "user %s is not in server !\n", name_buf);
			}
		}

		if (items[1].revents & ZMQ_POLLIN) {
			if (OPER == current_type) {
				oper = USER_ILLEGAL_OPER;
				int recv_len = zmq_recv(user_info_subscriber, &oper, OPER_LEN, 0); {
					if (OPER_LEN != recv_len) {
						oper = USER_ILLEGAL_OPER;
					}
				}
				current_type = IMEI;
			} else if (IMEI == current_type) {
				memset(imei, 0, strlen(imei));
				int recv_len = zmq_recv(user_info_subscriber, imei, IMEI_LEN, 0);

				// 如果长度不为15,则收到的imei是有问题的，对有问题的imei进行内存清空，内存清空之后就不会再进行用户插入删除操作了
				if (IMEI_LEN != recv_len) {
					memset(imei, 0, strlen(imei));
				}

				current_type = CHANNEL;
			} else if (CHANNEL == current_type) {
				memset(channel, 0, strlen(channel));
				int recv_len = zmq_recv(user_info_subscriber, channel, MAX_CHANNEL_NAME_LEN, 0);

				if ((NULL != imei) && (NULL != channel) && ((USER_ADD_CHANNEL == oper) || (USER_ADD_CHANNEL == oper))) {
					mfptp_edit_user_channel_info(oper, imei, channel);
				}

				current_type = OPER;
			}

			continue;
		}
	}

	zmq_close(subscriber);
	zmq_close(user_info_subscriber);
	zmq_ctx_destroy(context);
	return NULL;
}

