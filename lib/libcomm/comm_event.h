/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_EVENT_H__
#define __COMM_EVENT_H__

#include "comm_structure.h"
#include "comm_data.h"

#ifdef __cplusplus
extern "C" {
#endif


/* 初始化事件结构体 */
bool commevent_init(struct comm_event **commevent, struct comm_context *commctx);

/* 销毁一个事件结构体 */
void commevent_destroy(struct comm_event *commevent);

/* 接收新用户的连接 @fdidex: 监听fd在结构体struct listenfd数组中的下标 */
void commevent_accept(struct comm_event *commevent, int fdidx);

/* 处理残留的fd,epoll_wait超时和主循环里面调用 @timeout:是否是超时的时候调用该函数 */
void commevent_remainfd(struct comm_event *commevent, bool timeout);





#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_EVENT_H__ */
