/*********************************************************************************************/
/************************	Created by 许莉 on 16/02/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "comm_structure.h"

#ifdef __cplusplus
extern "C" {
#endif

enum fd_status{
	FD_WRITE = 0x01,
	FD_READ,
	FD_ACCEPT,
	FD_CLOSE,
	FD_TIMEOUT
};

/* 创建一个通信上下文的结构体 */
struct comm_context* comm_ctx_create(int epollsize);

/* 销毁一个通信上下文的结构体 */
void comm_ctx_destroy(struct comm_context *commctx);

/* bind或者connect某个指定的地址端口 */
int comm_socket(struct comm_context *commctx, char *host, char *service, struct cbinfo *finishedcb, int type);

/* 发送数据 */
int comm_send(struct comm_context *commctx, const struct comm_message *message);

/* 接收数据 */
int comm_recv(struct comm_context *commctx, struct comm_message *message);

/* 关闭套接字 */
void comm_close(struct comm_context *commctx, int fd);

/* 设置epoll_wait的超时时间 */
void comm_settimeout(struct comm_context *commctx, int timeout, CommCB callback, void *usr);



#ifdef __cplusplus
	}
#endif

#endif /* ifndef __COMMUNICATION_H__ */
