/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/7.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_epoll.h"

#define MAXEPOLLSIZE (1024*1000)	/* epoll允许检测的最大的描述符数 */

bool commepoll_init(struct comm_epoll *commepoll, int epollsize)
{
	assert(commepoll);
	memset(commepoll, 0, sizeof(*commepoll));

	commepoll->epollsize = (epollsize <= 0 || epollsize > MAXEPOLLSIZE) ? MAXEPOLLSIZE : epollsize;
	log("commepoll->epollsize:%d\n", commepoll->epollsize);
	commepoll->epfd = epoll_create(commepoll->epollsize);

	if (commepoll->epfd > -1) {
		NewArray(commepoll->events, commepoll->epollsize);

		if (commepoll->events) {
			commepoll->init = true;
			return true;
		}
	}

	return false;
}

void commepoll_destroy(struct comm_epoll *commepoll)
{
	if (commepoll && commepoll->init) {
		close(commepoll->epfd);
		Free(commepoll->events);
		commepoll->init = false;
	}
}

bool commepoll_add(struct comm_epoll *commepoll, int fd, int flag)
{
	assert(commepoll && commepoll->init);

	if (commepoll->watchcnt < commepoll->epollsize) {
		commepoll->events[0].data.fd = fd;
		commepoll->events[0].events = flag;

		if (!epoll_ctl(commepoll->epfd, EPOLL_CTL_ADD, fd, &commepoll->events[0])) {
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

	if (!epoll_ctl(commepoll->epfd, EPOLL_CTL_DEL, fd, &commepoll->events[0])) {
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

	if (!epoll_ctl(commepoll->epfd, EPOLL_CTL_MOD, fd, &commepoll->events[0])) {
		return true;
	} else {
		return false;
	}
}

#if 0
bool commepoll_wait(struct comm_epoll *commepoll, int timeout)
{
	assert(commepoll && commepoll->init);
	memset(commepoll->events, 0, (sizeof(struct epoll_event)) * commepoll->epollsize);
	commepoll->eventcnt = epoll_wait(commepoll->epfd, commepoll->events, commepoll->watchcnt, timeout);

	if (commepoll->eventcnt > 0) {
		return true;
	} else {
		return false;
	}
}
#endif

bool commepoll_wait(struct comm_epoll *commepoll, int timeout)
{
	assert(commepoll && commepoll->init);
	bool flag = false;
	memset(commepoll->events, 0, (sizeof(struct epoll_event)) * commepoll->epollsize);
	commepoll->eventcnt = epoll_wait(commepoll->epfd, commepoll->events, commepoll->watchcnt, timeout);

	if (commepoll->eventcnt > 0) {
		flag = true;
	} else if (commepoll->eventcnt == 0) {
		/* 超时 */
		errno = EINTR;
	}
	return flag;
}
