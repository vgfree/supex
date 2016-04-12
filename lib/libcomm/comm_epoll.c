/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/7.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_epoll.h"

inline int  create_epoll(int epollsize)
{
	return epoll_create(epollsize > 0 ? epollsize : EPOLLSIZE);
}

inline bool add_epoll(int epfd, int fd, unsigned int flag)
{
	int			retval = 0;
	struct epoll_event	event = {};

	event.data.fd = fd;
	event.events = flag;
	retval = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
	return retval == 0; 
}

inline bool mod_epoll(int epfd, int fd, unsigned int flag)
{
	int			retval = 0;
	struct epoll_event	event = {};
	event.data.fd = fd;
	event.events = flag;
	retval = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
	return retval == 0;
}

inline bool  del_epoll(int epfd, int fd, unsigned int flag)
{
	int			retval = 0;
	struct epoll_event	event = {};
	event.data.fd = fd;
	event.events = EPOLLIN;
	retval = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
	return retval == 0;
}

inline int  wait_epoll(int epfd, struct epoll_event* events, int watchcnt, int timeout)
{
	return epoll_wait(epfd, events, watchcnt, timeout);
}
