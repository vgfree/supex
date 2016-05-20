/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_tcp.h"

#define  LISTENFDS  1024	/* 能够监听的描述符的个数 */

#define	 CLOSEFD(fd)		({ close(fd);	\
				   fd = -1;	})

static bool _get_portinfo(struct comm_tcp* commtcp);

static bool _get_addrinfo(struct addrinfo** ai, const char* host, const char* service);

static bool _bind_listen(struct comm_tcp* commtcp, struct addrinfo* ai);

static bool _connect(struct comm_tcp *commtcp, struct addrinfo *ai);


bool socket_listen(struct comm_tcp* commtcp, const char* host, const char* service)
{
	assert(commtcp && host && service);
	memset(commtcp, 0, sizeof(*commtcp));

	struct addrinfo*	ai = NULL;
	if (_get_addrinfo(&ai, host, service)) {
		if (_bind_listen(commtcp, ai)) {
			if (_get_portinfo(commtcp)) {
				commtcp->type = COMM_BIND;
				commtcp->stat = FD_INIT;
			} else {
				CLOSEFD(commtcp->fd);
			} 
		}
		freeaddrinfo(ai);
	} else {
		commtcp->fd = -1;
	}
	return (commtcp->fd != -1);
}

bool socket_connect(struct comm_tcp* commtcp, const char* host, const char* service)
{
	assert(commtcp && host && service);
	memset(commtcp, 0, sizeof(*commtcp));

	struct addrinfo*	ai = NULL;
	if (_get_addrinfo(&ai, host, service)) {
		if (_connect(commtcp, ai)) {
			if (_get_portinfo(commtcp)) {
				commtcp->type = COMM_CONNECT;
				commtcp->stat = FD_INIT;
			} else {
				CLOSEFD(commtcp->fd);
			}
		}
		freeaddrinfo(ai);
	} else {
		commtcp->fd = -1;
	}
	return (commtcp->fd != -1);
}

int socket_accept(const struct comm_tcp* lsncommtcp, struct comm_tcp* acptcommtcp)
{
	assert(lsncommtcp && acptcommtcp);
	memset(acptcommtcp, 0, sizeof(*acptcommtcp));

	acptcommtcp->fd = accept(lsncommtcp->fd, NULL, NULL);
	if (acptcommtcp->fd > 0) {
		if (fd_setopt(acptcommtcp->fd, O_NONBLOCK)) {
			if (_get_portinfo(acptcommtcp)) {
				acptcommtcp->type = COMM_ACCEPT;
				acptcommtcp->stat = FD_INIT;
				return acptcommtcp->fd;	/* 成功接收到新连接并设置完毕 */
			} else {
				CLOSEFD(acptcommtcp->fd);
			}
		} else {
			CLOSEFD(acptcommtcp->fd);
		}
	} else {
		return -1;	/* -1代表accept出错，可以检测errno*/
	}

	return -2;	/* 代表设置出错 */
}

/* 获取端口相关信息:获得是本地字节序 */
static bool _get_portinfo(struct comm_tcp* commtcp)
{
	assert(commtcp && commtcp->fd > 0);

	int		retval		= -1;
	const char*	pointer		= NULL;
	struct sockaddr sockaddr	= {};
	socklen_t	len		= sizeof(sockaddr);
	size_t		plen		= sizeof(commtcp->addr);

	if (!getsockname(commtcp->fd, &sockaddr, &len)) {
		switch (sockaddr.sa_family)
		{
			case AF_INET: {
					struct sockaddr_in *inaddr = (struct sockaddr_in*)&sockaddr;
					pointer = inet_ntop(AF_INET, &inaddr->sin_addr, commtcp->addr, plen);
					commtcp->port = inaddr->sin_port;
				      }
					break;

			case AF_INET6: {
					struct sockaddr_in6 *inaddr6 = (struct sockaddr_in6*)&sockaddr;
					pointer = inet_ntop(AF_INET6, &inaddr6->sin6_addr, commtcp->addr, plen);
					commtcp->port = inaddr6->sin6_port;
				       }
					break;

			case AF_UNIX: {
					struct sockaddr_un *unaddr = (struct sockaddr_un*)&sockaddr;
					snprintf(commtcp->addr, plen, "%s", unaddr->sun_path);
					pointer = commtcp->addr;
				      }
					break;

			default:
				break;
		}
	} 

	if (pointer != NULL) {
		return true;
	} else {
		return false;
	}
}

/* 获取地址信息 */
static bool _get_addrinfo(struct addrinfo** ai, const char* host, const char* service)
{
	assert(ai && host && service);
	int		retval= -1;
	struct addrinfo	hints = {};

	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	do {
		retval = getaddrinfo(host, service, &hints, ai);
		if (likely(retval == 0)) {
			/* 函数成功返回 */
			return true;
		} else if (likely(retval == EAI_AGAIN)) {
			continue ;
		} else {
			return false;
		}
	} while(1);
}

/* 根据地址信息绑定和监听 */
static bool _bind_listen(struct comm_tcp* commtcp, struct addrinfo* ai)
{
	int		 optval = 0;
	bool		 flag = false;
	struct addrinfo* aiptr = NULL;
	for (aiptr = ai; aiptr != NULL; aiptr = aiptr->ai_next) {
		commtcp->fd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
		if (commtcp->fd > 0) {
			if (bind(commtcp->fd, aiptr->ai_addr, aiptr->ai_addrlen) == 0) {
				if (unlikely(listen(commtcp->fd, LISTENFDS) == -1)) {	/* 监听描述符 */
					CLOSEFD(commtcp->fd);
					break ;
				}
				if (unlikely(setsockopt(commtcp->fd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)sizeof(int)))) {	/* 设置地址可重用 */
					CLOSEFD(commtcp->fd);
					break ;
				}
				if (unlikely(!fd_setopt(commtcp->fd, O_NONBLOCK))) {	/* 将套接字设置为非阻塞模式 */
					CLOSEFD(commtcp->fd);
					break ;
				}
				flag = true;
				break ;
			} else {						/* 绑定失败 忽略此描述符,继续循环下一个 */
				CLOSEFD(commtcp->fd);
			}
		}
	}
	return flag;
}


static inline bool _connect(struct comm_tcp *commtcp, struct addrinfo *ai) 
{
	assert(commtcp && ai);
	bool		 flag = false;
	struct addrinfo* aiptr = NULL;

	for (aiptr = ai; aiptr != NULL; aiptr = aiptr->ai_next) {
		commtcp->fd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
		if (commtcp->fd > 0) {
			if (connect(commtcp->fd, aiptr->ai_addr, aiptr->ai_addrlen) == 0) {
				if (fd_setopt(commtcp->fd, O_NONBLOCK)) {	/* 将套接字设置为非阻塞模式 */
					flag = true;
				} else {
					CLOSEFD(commtcp->fd);
				}
				break ;
			} else {						/* 连接失败 忽略此描述符,继续循环下一个 */
				CLOSEFD(commtcp->fd);
			}
		}
	}
	return flag;
}
