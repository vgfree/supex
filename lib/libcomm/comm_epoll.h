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

#define EPOLLSIZE	1024

/* epoll_wait 发生的事件类型 */
enum epoll_events {
	EPOLL_WRITE = 0x01,	/* 发生读事件 */
	EPOLL_READ,		/* 发生写事件 */
	EPOLL_ACCEPTED,		/* 发生新用户连接事件 */
	EPOLL_TIMEOUTED		/* 发生超时事件 */
};

/* epoll_wait 相关的参数设置 */
struct epoll_args {
	int listenfd;		/* 不为-1的时候则需要判断是否是新用户连接 */
	int watchcnt;		/* 此时监控的fd的个数 */
	int timeout;		/* epoll_wait超时的时间 */
};

/* 创建一个epoll */
int  create_epoll(int epollsize);

/* 往epfd里面添加一个需要监控的fd */
bool add_epoll(int epfd, int fd, unsigned int flag);

/* 修改一个epfd里面的已经监控的fd事件 */
bool mod_epoll(int epfd, int fd, unsigned int flag);

/* 从epfd里面删除一个已经监控的fd */
bool del_epoll(int epfd, int fd, unsigned int flag);

/* epoll等待事件的发生 */
int  wait_epoll(int epfd, struct epoll_event* events, int watchcnt, int timeout);

//inline int  wait_epoll(int epfd, struct epoll_args args, enum epoll_events *event);

     
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef _COMM_EPOLL_H_*/

