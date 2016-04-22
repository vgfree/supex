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


/* 创建一个通信上下文的结构体 */
struct comm_context* comm_ctx_create(int epollsize);

/* 销毁一个通信上下文的结构体 */
void comm_ctx_destroy(struct comm_context *commctx);

/* bind或者connect某个指定的地址端口 */
int comm_socket(struct comm_context *commctx, const char *host, const char *service, struct cbinfo *finishedcb, int type);

/* @block:发送数据失败的时候是否阻塞等待 @timeout[单位：ms]：阻塞多长时间返回，-1为一直阻塞 @返回值为-1失败*/
int comm_send(struct comm_context *commctx, const struct comm_message *message, bool block, int timeout);

/* @block:接收数据失败的时候是否阻塞等待 @timeout[单位：ms]：阻塞多长时间返回，-1为一直阻塞 @返回值为-1失败*/
int comm_recv(struct comm_context *commctx, struct comm_message *message, bool block, int timeout);

/* 关闭指定套接字 */
void comm_close(struct comm_context *commctx, int fd);

/* 设置epoll_wait的超时时间 */
void comm_settimeout(struct comm_context *commctx, int timeout, CommCB callback, void *usr);



#ifdef __cplusplus
	}
#endif

#endif /* ifndef __COMMUNICATION_H__ */
