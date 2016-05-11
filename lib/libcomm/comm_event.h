/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_EVENT_H__
#define __COMM_EVENT_H__

#include "comm_structure.h"
#include "comm_parse.h"
#include "comm_package.h"
#include "communication.h"

#ifdef __cplusplus
extern "C" {
#endif


/* epoll_wait超时时的回调函数:解析和打包数据 */
void timeout_event(struct comm_context* commctx);

/* 接收新用户的连接 */
void accept_event(struct comm_context *commctx);

/* 有数据可以接收的时候 */
void recv_event(struct comm_context *commctx, int fd);

/* 有数据可以发送的时候 */
void send_event(struct comm_context *commctx, int *fda, int cnt);



#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_EVENT_H__ */
