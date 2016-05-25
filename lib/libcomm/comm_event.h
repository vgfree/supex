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

/* 检测是否fd是为COMM_BIND类型的fd并保存于struct listenfd结构体中的数组中  @返回值：-1代表不是，否则返回fd所在数组的下标 */
static inline int gain_listenfd_fdidx(struct listenfd *listenfd, int fd)
{
	assert(listenfd && fd > 0);
	int fdidx = 0;
	for (fdidx = 0; fdidx < listenfd->counter; fdidx++) {
		if (fd == listenfd->commtcp[fdidx].fd) {
			return fdidx;
		}
	}
	return -1;
}


/* 初始化事件结构体 */
bool commevent_init(struct comm_event** commevent, struct comm_context* commctx);

/* 销毁一个事件结构体 */
void commevent_destroy(struct comm_event* commevent);

/* 往结构体struct comm_event里面增加一个fd相关信息  */
bool commevent_add(struct comm_event *commevent, struct comm_tcp *commtcp, struct cbinfo *finishedcb);

/* 从结构体struct comm_event里面删除一个fd的相关信息 */
void commevent_del(struct comm_event *commevent, int fd);

/* epoll_wait超时时的回调函数:解析和打包数据 */
void commevent_timeout(struct comm_event* commevent);

/* 接收新用户的连接 @fdidex: 监听fd在结构体struct listenfd数组中的下标 */
void commevent_accept(struct comm_event* commevent, int fdidx);

/* 有数据可以接收的时候 */
void commevent_recv(struct comm_event* commevent, int fd);

/* 有数据可以发送的时候 */
void commevent_send(struct comm_event* commevent, int* fda, int cnt);



#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_EVENT_H__ */
