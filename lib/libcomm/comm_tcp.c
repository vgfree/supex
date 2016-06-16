/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_tcp.h"
#include <sys/time.h>

#define  LISTENFDS  1024	/* 能够监听的描述符的个数 */
#define	 TIMEOUTTIME  5		/* select的超时事件[单位:s],超时就判定connect连接失败 */

#define	 CLOSEFD(fd)		({		\
				   close(fd);	\
				   fd = -1;	\
				})

static bool _get_portinfo(struct comm_tcp *commtcp, bool local);

static bool _get_addrinfo(struct addrinfo **ai, const char *host, const char *service);

static bool _bind_listen(struct comm_tcp *commtcp, struct addrinfo *ai);

static bool _start_connect(struct comm_tcp *commtcp, struct addrinfo *ai, int timeout);

static bool _connect(struct comm_tcp *commtcp, struct addrinfo *ai);


bool socket_listen(struct comm_tcp *commtcp, const char *host, const char *service)
{
	assert(commtcp && host && service);

	struct addrinfo* ai = NULL;

	memset(commtcp, 0, sizeof(*commtcp));
	commtcp->localport = atoi(service);
	memcpy(commtcp->localaddr, host, strlen(host));
	if (_get_addrinfo(&ai, host, service)) {
		if (_bind_listen(commtcp, ai)) {
			commtcp->type = COMM_BIND;
			commtcp->stat = FD_INIT;
		}
		freeaddrinfo(ai);
	} else {
		commtcp->fd = -1;
	}
	return (commtcp->fd != -1);
}

bool socket_connect(struct comm_tcp *commtcp, const char *host, const char *service, int timeout, int connattr)
{
	assert(commtcp && host && service);

	struct addrinfo* ai = NULL;

	memset(commtcp, 0, sizeof(*commtcp));
	commtcp->peerport = atoi(service);
	memcpy(commtcp->peeraddr, host, strlen(host));
	if (_get_addrinfo(&ai, host, service)) {
		commtcp->connattr = connattr;
		if (_start_connect(commtcp, ai, timeout)) {
			if (_get_portinfo(commtcp, true)) {
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

int socket_accept(const struct comm_tcp *lsncommtcp, struct comm_tcp *acptcommtcp)
{
	assert(lsncommtcp && lsncommtcp->fd > 0 && acptcommtcp);

	memset(acptcommtcp, 0, sizeof(*acptcommtcp));
	acptcommtcp->fd = accept(lsncommtcp->fd, NULL, NULL);
	if (acptcommtcp->fd > 0) {
		if (fd_setopt(acptcommtcp->fd, O_NONBLOCK)) {
			if (_get_portinfo(acptcommtcp, true) && _get_portinfo(acptcommtcp, false)) {
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

	return -2;	/* 代表设置出错,直接忽略错误 */
}

/*获取端口信息 @local:true 为获取本地的IP地址和端口号， false：获取对端的IP地址和端口号 */
static bool _get_portinfo(struct comm_tcp *commtcp, bool local)
{
	assert(commtcp && commtcp->fd > 0);

	char*		addr		= NULL;
	uint16_t*	port		= NULL;
	const char*	pointer		= NULL;
	struct sockaddr sockaddr	= {};
	socklen_t	len		= sizeof(sockaddr);
	size_t		plen		= sizeof(commtcp->localaddr);

	if (local) {
		if (getsockname(commtcp->fd, &sockaddr, &len) == 0) {
			port = &commtcp->localport;
			addr = commtcp->localaddr;
		} else {
			return false;
		}
	} else {
		if (getpeername(commtcp->fd, &sockaddr, &len) == 0) {
			port = &commtcp->peerport;
			addr = commtcp->peeraddr;
		} else {
			return false;
		}
	}
	switch (sockaddr.sa_family)
	{
		case AF_INET: {
				struct sockaddr_in *inaddr = (struct sockaddr_in*)&sockaddr;
				pointer = inet_ntop(AF_INET, &inaddr->sin_addr, addr, plen);
				*port = inaddr->sin_port;
			      }
				break;

		case AF_INET6: {
				struct sockaddr_in6 *inaddr6 = (struct sockaddr_in6*)&sockaddr;
				pointer = inet_ntop(AF_INET6, &inaddr6->sin6_addr, addr, plen);
				*port = inaddr6->sin6_port;
			       }
				break;

		case AF_UNIX: {
				struct sockaddr_un *unaddr = (struct sockaddr_un*)&sockaddr;
				snprintf(addr, plen, "%s", unaddr->sun_path);
				pointer = addr;
			      }
				break;

		default:
			break;
	}

	return pointer != NULL ? true : false;
}


/* 获取地址信息 */
static bool _get_addrinfo(struct addrinfo **ai, const char *host, const char *service)
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
static bool _bind_listen(struct comm_tcp *commtcp, struct addrinfo *ai)
{
	assert(commtcp && ai);

	int		 optval = SO_REUSEADDR;
	bool		 flag = false;
	struct addrinfo* aiptr = NULL;

	for (aiptr = ai; aiptr != NULL; aiptr = aiptr->ai_next) {
		commtcp->fd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
		if (commtcp->fd > 0) {
			if (setsockopt(commtcp->fd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)sizeof(int)) == 0) { /* 设置地址可重用 */
				if (bind(commtcp->fd, aiptr->ai_addr, aiptr->ai_addrlen) == 0) {
					if (unlikely(listen(commtcp->fd, LISTENFDS) == -1)) {	/* 监听描述符 */
						CLOSEFD(commtcp->fd);
						continue ;
					}
					if (unlikely(!fd_setopt(commtcp->fd, O_NONBLOCK))) {	/* 将套接字设置为非阻塞模式 */
						CLOSEFD(commtcp->fd);
						continue ;
					}
					flag = true;
					break ;
				}
				//log("%d\n", errno);
			}
			CLOSEFD(commtcp->fd);	/* 发生错误，忽略此描述符，继续下一个描述符 */
		}
	}
	return flag;
}


static bool _start_connect(struct comm_tcp *commtcp, struct addrinfo *ai, int timeout) 
{
	assert(commtcp && ai);

	struct addrinfo* aiptr = NULL;
	struct timeval  start = {};

	if (timeout > 0) {
		gettimeofday(&start, NULL);
	}

	for (aiptr = ai; aiptr != NULL; aiptr = aiptr->ai_next) {
		commtcp->fd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
		if (commtcp->fd > 0) {
			if (fd_setopt(commtcp->fd, O_NONBLOCK)) {
				while (!_connect(commtcp, aiptr)) {
					/* 连接失败 */
					if (commtcp->connattr == CONNECT_ONCE || timeout == 0) {
						/* 只允许尝试连接一次 */
						CLOSEFD(commtcp->fd);
						return false;
					} else {
						/* 尝试重复连接，判断是否超时 */
						if (timeout > 0) {
							struct timeval  end = {};
							long            diffms = 0;
							gettimeofday(&end, NULL);
							diffms = (end.tv_sec - start.tv_sec) * 1000;
							diffms += (end.tv_usec - start.tv_usec) / 1000;
							if (timeout - diffms < 1) {
								log("try connect to peer faile because of timeout\n");
								CLOSEFD(commtcp->fd);
								return false;
							}
						}
					}
				}
				return true;
			}
			CLOSEFD(commtcp->fd);	/* 发生其他的任何错误，都忽略此描述符，继续下一个 */
		}
	}
	return false;
}

static bool _connect(struct comm_tcp *commtcp, struct addrinfo *aiptr)
{
	bool flag = false;
	if (connect(commtcp->fd, aiptr->ai_addr, aiptr->ai_addrlen) == -1) {
		if (errno == EINPROGRESS ) {
			/* 连接正在进程中 检测是否会连接成功 */
			fd_set set;
			int error = -1;
			int retval = -1;
			int len = sizeof(int);
			struct timeval tm;
			FD_ZERO(&set);
			FD_SET(commtcp->fd, &set);
			tm.tv_sec = TIMEOUTTIME;
			tm.tv_usec = 0; 
			if (select(commtcp->fd+1, NULL, &set, NULL, &tm) > 0) {
				retval = getsockopt(commtcp->fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
				if (retval < 0 || error) {
					/*失败 Solaris版本将retval设为-1,Berkeley版本返回0,设置error错误值 */
					if (error == EHOSTUNREACH || errno == EHOSTUNREACH) {
						/* 错误值为此，则代表对端端口未打开 */
						//log("%s\n", strerror(errno));
						log("peer port isn't open\n");
					}
				}  else {
					/* 连接成功 */
					flag = true;
				}
			}
		} 
	} else {
		flag = true;
	}

	return flag;
}
