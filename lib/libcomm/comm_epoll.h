/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/7.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_EPOLL_H__
#define __COMM_EPOLL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "comm_utils.h"
#include <sys/epoll.h>

#define MAXEPOLLSIZE	1024


struct comm_epoll {
	bool			init;			/* 结构体是否已经初始化 */
	int			epfd;			/* epoll的句柄 */
	int			epollsize;		/* epoll能够监听事件的最大值 */
	int			watchcnt;		/* 正在监听的fd的个数 */
	int			eventcnt;		/* 被触发的事件的个数 */
	struct epoll_event	*events;		/* 保存epoll监听到的事件 */
};

/* 初始化epoll结构体 */
bool commepoll_init(struct comm_epoll *commepoll, int epollsize);

/* 销毁epoll结构体 */
void commepoll_destroy(struct comm_epoll *commepoll);

/* 往epfd里面添加一个需要监控的fd */
bool commepoll_add(struct comm_epoll *commepoll, int fd, int flag);

/* 修改一个epfd里面的已经监控的fd事件 */
bool commepoll_mod(struct comm_epoll *commepoll, int fd, int flag);

/* 从epfd里面删除一个已经监控的fd */
bool commepoll_del(struct comm_epoll *commepoll, int fd, int flag);

/* epoll等待事件的发生 */
bool commepoll_wait(struct comm_epoll *commepoll, int timeout);


     
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef _COMM_EPOLL_H_*/
