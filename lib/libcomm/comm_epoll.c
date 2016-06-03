/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/7.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_epoll.h"


#define MAXEPOLLSIZE	1024	/* epoll允许检测的最大的描述符数 */

bool commepoll_init(struct comm_epoll *commepoll, int epollsize)
{
	assert(commepoll);
	memset(commepoll, 0, sizeof(*commepoll));
	commepoll->epollsize = (epollsize <= 0 || epollsize > MAXEPOLLSIZE) ? MAXEPOLLSIZE : epollsize;
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
	return ;
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
		return false ;
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
	int n = 0, i = 0, cnt = 0;
	memset(commepoll->events, 0, (sizeof(struct epoll_event)) * commepoll->epollsize);
	commepoll->eventcnt = epoll_wait(commepoll->epfd, commepoll->events, commepoll->watchcnt, timeout);
	if (commepoll->eventcnt > 0) {
#if	0 
		for (n = 0; n < commepoll->eventcnt; n++) {
			if (commepoll->events[n].data.fd > 0) {
				/* 新的客户端连接， 触发accept事件 */
				if ((fdidx = gain_listenfd_fdidx(&commctx->commevent->listenfd, commctx->commepoll.events[n].data.fd)) > -1) {
					 //commevent_accept(commctx->commevent, fdidx);
					 /* 将fd所在的索引号保存进去而不是fd */ 
					add_remainfd(&commevent->remainfd, fdidx, REMAINFD_LISTEN);

				} else if (commepoll->events[n].events & EPOLLIN) {			/* 有数据可读，触发读数据事件 */
					/* 管道事件被触发，则开始写事件 */
					if (commepoll->events[n].data.fd == commevent->commctx->commpipe.rfd) {
						int array[EPOLL_SIZE] = {};
						if ((cnt = commpipe_read(&commevent->commctx->commpipe, array, sizeof(int))) > 0) {
							for (i = 0; i < cnt; i++) {
								add_remainfd(&commevent->remainfd, array[i], REMAINFD_WRITE);
							}
						//	_set_remainfd(&commevent->remainfd, array, cnt);
							//commevent_send(commctx->commevent, remainfd->wfda[remainfd->wcnt-1], true);
						}
					} else {	/* 非pipe的fd读事件被触发，则代表是socket的fd触发了读事件 */
						add_remainfd(&commevent->remainfd, fd, REMAINFD_READ);
						// commevent_recv(commctx->commevent, commctx->commepoll.events[n].data.fd, false);
					}
				} else if (commepoll->events[n].events & EPOLLOUT) {			/* 有数据可写， 触发写数据事件 */
					//commevent_send(commctx->commevent, commctx->commepoll.events[n].data.fd, false);
					add_remainfd(&commevent->remainfd, fd, REMAINFD_WRITE);
				}
			}
		}
#endif
		return true;
	}
	return false;
}
