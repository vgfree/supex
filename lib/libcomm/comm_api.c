#include <dirent.h>
#include <stdlib.h>
#include "comm_api.h"

/*************************************** 数据结构体相关操作 内部使用 ********************************************/

/* 一个新的线程开始运行 */
static void *_start_main_loop(void *usr)
{
	struct comm_context *commctx = (struct comm_context *)usr;

	assert(commctx);

	/* 状态设置为COMM_STAT_RUN，唤醒等待线程 */
	commlock_lock(&commctx->statlock);
	commctx->stat = COMM_STAT_RUN;
	commlock_wake(&commctx->statlock);
	commlock_unlock(&commctx->statlock);

	do {
		/* 线程状态为STOP的时候则将状态设置为NONE，返回真，则代表设置成功，退出循环 */
		if (unlikely(commctx->stat == COMM_STAT_STOP)) {
			ATOMIC_SET(&commctx->stat, COMM_STAT_NONE);
			break;
		}

		commevts_once(&commctx->commevts);
	} while (1);
	return NULL;
}

struct comm_context *commapi_ctx_create(void)
{
	struct comm_context *commctx = NULL;

	New(commctx);

	if (unlikely(!commctx)) {
		goto error;
	}

	/* evts init */
	if (unlikely(!commevts_make(&commctx->commevts))) {
		goto error;
	}

	commctx->commevts.commctx = commctx;

	/* stat init */
	commctx->stat = COMM_STAT_INIT;

	if (unlikely(!commlock_init(&commctx->statlock))) {
		goto error;
	}

	if (unlikely((pthread_create(&commctx->ptid, NULL, _start_main_loop, (void *)commctx))) != 0) {
		goto error;
	}

	commlock_lock(&commctx->statlock);
	do {
		if (commctx->stat == COMM_STAT_RUN) {
			break;
		}
		commlock_wait(&commctx->statlock, -1);
	} while (1);
	commlock_unlock(&commctx->statlock);

	return commctx;

error:

	if (commctx) {
		commevts_free(&commctx->commevts);

		commlock_destroy(&commctx->statlock);
		Free(commctx);
	}

	return NULL;
}

void commapi_ctx_destroy(struct comm_context *commctx)
{
	if (commctx) {
		ATOMIC_SET(&commctx->stat, COMM_STAT_STOP);

		/* 等待子线程退出然后再继续销毁数据 */
		pthread_join(commctx->ptid, NULL);
		assert(commctx->stat == COMM_STAT_NONE);

		/*释放事件及数据*/
		commevts_free(&commctx->commevts);

		commlock_destroy(&commctx->statlock);
		Free(commctx);
	}
}

int commapi_socket(struct comm_context *commctx, const char *host, const char *port, struct comm_cbinfo *cbinfo, int stype)
{
	assert(commctx && host && port);

	struct comm_tcp commtcp = {};

	if (stype == COMM_BIND) {
		if (commctx->commevts.bindfdcnt >= LISTEN_SIZE) {
			loger("bind too many socket in one comm_context\n");
			return -1;
		}

		if (unlikely(rsocket_open(&commtcp.rsocket, host, port))) {
			loger("create socket failed\n");
			return -1;
		}
		if (unlikely(rsocket_bind_and_listen(&commtcp.rsocket))) {
			loger("bind socket failed\n");
			return -1;
		}
		commtcp.type = COMM_BIND;
		commtcp_set_portinfo(&commtcp, true, host, port);
	} else {
		if (commctx->commevts.connfdcnt >= EPOLL_SIZE) {
			loger("conn too many socket in one comm_context\n");
			return -1;
		}

		if (unlikely(rsocket_open(&commtcp.rsocket, host, port))) {
			loger("create socket failed\n");
			return -1;
		}
		commtcp.type = COMM_CONNECT;
		commtcp_set_portinfo(&commtcp, false, host, port);
	}

	bool ok = commevts_socket(&commctx->commevts, &commtcp, cbinfo);

	if (!ok) {
		rsocket_close(&commtcp.rsocket);
		loger("add socket fd to monitor failed\n");
		return -1;
	}

	return commtcp.rsocket.sktfd;
}

int commapi_send(struct comm_context *commctx, struct comm_message *message)
{
	assert(commctx && message);
	assert(message->fd > 0 && message->fd < EPOLL_SIZE);	/* 保证描述符在范围内 */

	if (unlikely(!commmsg_check(message))) {
		return -1;
	}

	bool ok = commevts_push(&commctx->commevts, message);
	return ok ? 0 : -1;
}

int commapi_recv(struct comm_context *commctx, struct comm_message *message)
{
	assert(commctx && message);

	bool ok = commevts_pull(&commctx->commevts, message);
	return ok ? 0 : -1;
}

void commapi_close(struct comm_context *commctx, int fd)
{
	commevts_close(&commctx->commevts, fd);
}

