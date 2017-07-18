#pragma once

#include "comm_message.h"
#include "comm_tcp.h"
#include "comm_dispose_evts.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** munication模块 ************************************/

/* 通信模块的上下文环境结构体 */
struct comm_context
{
	pthread_t               ptid;			/* 新线程的pid */

	struct comm_evts        commevts;		/* 事件驱动的相关信息 */

	enum
	{
		COMM_STAT_NONE,
		COMM_STAT_INIT,
		COMM_STAT_RUN,
		COMM_STAT_STOP
	}                       stat;			/* 线程的状态 */
	struct comm_lock        statlock;		/* 用来同步stat的状态 */
};

/* 创建一个通信上下文的结构体 */
struct comm_context     *commapi_ctx_create(void);

/* 销毁一个通信上下文的结构体 */
void commapi_ctx_destroy(struct comm_context *commctx);

/*********************************************************************************/

/* bind或者connect某个指定的地址端口 @stype:可取的值为COMM_BIND或COMM_CONNECT*/
int commapi_socket(struct comm_context *commctx, const char *host, const char *port, struct cbinfo *finishedcb, int stype);

/* @返回值为-1失败*/
int commapi_send(struct comm_context *commctx, struct comm_message *message);

/* @返回值为-1失败*/
int commapi_recv(struct comm_context *commctx, struct comm_message *message);

/* 关闭指定套接字 */
void commapi_close(struct comm_context *commctx, int fd);

#ifdef __cplusplus
}
#endif

