/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/7.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_epoll.h"

bool commepoll_init(struct comm_epoll *commepoll, int epollsize)
{
	assert(commepoll);
	memset(commepoll, 0, sizeof(*commepoll));
	if (epollsize > MAXEPOLLSIZE || epollsize < 1) {
		commepoll->epollsize = MAXEPOLLSIZE;
	} else {
		commepoll->epollsize = epollsize;
	}

	commepoll->epfd = epoll_create(commepoll->epollsize);
	if (likely(commepoll->epfd > -1)) {
		NewArray(commepoll->events, commepoll->epollsize);
		if (likely(commepoll->events)) {
			commepoll->init = true;
			return true;
		}
	}
	return false;
}

void commepoll_destroy(struct comm_epoll *commepoll)
{
	assert(commepoll && commepoll->init);
	close(commepoll->epfd);
	Free(commepoll->events);
	commepoll->init = false;
}

bool commepoll_add(struct comm_epoll *commepoll, int fd, int flag)
{
	assert(commepoll && commepoll->init);

	if (likely(commepoll->watchcnt < commepoll->epollsize)) {
		commepoll->events[0].data.fd = fd;
		commepoll->events[0].events = flag;
		if (likely(!epoll_ctl(commepoll->epfd, EPOLL_CTL_ADD, fd, &commepoll->events[0]))) {
			commepoll->watchcnt += 1;
			return true;
		}
	}

	return false;
}

bool commepoll_del(struct comm_epoll *commepoll, int fd, int flag)
{
	assert(commepoll && commepoll->init);
	
	commepoll->events[0].data.fd = fd;
	commepoll->events[0].events = EPOLLIN;
	if (likely(!epoll_ctl(commepoll->epfd, EPOLL_CTL_DEL, fd, &commepoll->events[0]))) {
		commepoll->watchcnt -= 1;
		return true;
	} else {
		return false;
	}
}

bool commepoll_mod(struct comm_epoll *commepoll, int fd, int flag)
{
	assert(commepoll && commepoll->init);
	commepoll->events[0].data.fd = fd;
	commepoll->events[0].events = flag;
	if (likely(!epoll_ctl(commepoll->epfd, EPOLL_CTL_MOD, fd, &commepoll->events[0]))) {
		return true;
	} else {
		return false ;
	}
}

bool commepoll_wait(struct comm_epoll *commepoll, int timeout)
{
	assert(commepoll && commepoll->init);
	memset(commepoll->events, 0, (sizeof(struct epoll_event)) * commepoll->epollsize);
	commepoll->eventcnt = epoll_wait(commepoll->epfd, commepoll->events, commepoll->watchcnt, timeout);
	if ( likely(commepoll->eventcnt != -1)) {
		return true;
	} else {
		return false;
	}
}
