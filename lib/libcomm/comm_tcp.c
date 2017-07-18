/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_tcp.h"
#include <sys/time.h>

#define  MAX_LISTENFDS  10240		/* 能够监听的描述符的个数 */
#define  TIMEOUTTIME    5		/* select的超时事件[单位:s],超时就判定connect连接失败 */
#define  CONNECTTIMEOUT 1000 * 5	/* 连接服务器时的标志位设置了CONNECT_ANYWAY时，一直尝试连接服务器,超时时间到还没连接上就返回[单位:ms] */

#define  CLOSEFD(fd)	   \
	({		   \
		close(fd); \
		fd = -1;   \
	})

static bool _get_portinfo(struct comm_tcp *commtcp, bool local);

static bool _get_addrinfo(struct addrinfo **ai, const char *host, const char *port);

static bool _bind_listen(struct comm_tcp *commtcp, struct addrinfo *ai);

static bool _start_connect(struct comm_tcp *commtcp, struct addrinfo *ai, int cnt_timeout);

bool socket_listen(struct comm_tcp *commtcp, const char *host, const char *port)
{
	assert(commtcp && host && port);

	memset(commtcp, 0, sizeof(*commtcp));
	commtcp->localport = atoi(port);
	memcpy(commtcp->localaddr, host, strlen(host));

	struct addrinfo *ai = NULL;

	if (_get_addrinfo(&ai, host, port)) {
		if (_bind_listen(commtcp, ai)) {
			commtcp->type = COMM_BIND;
			commtcp->stat = FD_INIT;
		}

		freeaddrinfo(ai);
	} else {
		commtcp->fd = -1;
	}

	return commtcp->fd != -1;
}

bool socket_connect(struct comm_tcp *commtcp, const char *host, const char *port, int retry)
{
	assert(commtcp && host && port);

	memset(commtcp, 0, sizeof(*commtcp));
	commtcp->peerport = atoi(port);
	memcpy(commtcp->peeraddr, host, strlen(host));

	struct addrinfo *ai = NULL;

	if (_get_addrinfo(&ai, host, port)) {
		while (!_start_connect(commtcp, ai, CONNECTTIMEOUT)) {
			if (retry == -1) {
				continue;
			}

			if (retry-- <= 0) {
				/* 重试次数用完 */
				loger("connect fatal error fd: %d errno:%d\n", commtcp->fd, errno);
				freeaddrinfo(ai);
				return false;
			}
		}

		if (_get_portinfo(commtcp, true)) {
			commtcp->type = COMM_CONNECT;
			commtcp->stat = FD_INIT;
		} else {
			CLOSEFD(commtcp->fd);
		}

		freeaddrinfo(ai);
	} else {
		commtcp->fd = -1;
	}

	return commtcp->fd != -1;
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
			}
		}

		CLOSEFD(acptcommtcp->fd);
		return -2;	/* 代表设置出错,直接忽略错误 */
	} else {
		return -1;	/* -1代表accept出错，可以检测errno*/
	}
}

/*获取端口信息 @local:true 为获取本地的IP地址和端口号， false：获取对端的IP地址和端口号 */
static bool _get_portinfo(struct comm_tcp *commtcp, bool local)
{
	assert(commtcp && commtcp->fd > 0);

	char            *addr = NULL;
	uint16_t        *port = NULL;
	const char      *pointer = NULL;
	struct sockaddr sockaddr = {};
	socklen_t       len = sizeof(sockaddr);
	size_t          plen = sizeof(commtcp->localaddr);

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
		case AF_INET:
		{
			struct sockaddr_in *inaddr = (struct sockaddr_in *)&sockaddr;
			pointer = inet_ntop(AF_INET, &inaddr->sin_addr, addr, plen);
			*port = inaddr->sin_port;
		}
		break;

		case AF_INET6:
		{
			struct sockaddr_in6 *inaddr6 = (struct sockaddr_in6 *)&sockaddr;
			pointer = inet_ntop(AF_INET6, &inaddr6->sin6_addr, addr, plen);
			*port = inaddr6->sin6_port;
		}
		break;

		case AF_UNIX:
		{
			struct sockaddr_un *unaddr = (struct sockaddr_un *)&sockaddr;
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
static bool _get_addrinfo(struct addrinfo **ai, const char *host, const char *port)
{
	assert(ai && host && port);

	struct addrinfo hints = {};
	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	do {
		int retval = getaddrinfo(host, port, &hints, ai);

		if (likely(retval == 0)) {
			/* 函数成功返回 */
			return true;
		} else if (likely(retval == EAI_AGAIN)) {
			continue;
		} else {
			return false;
		}
	} while (1);
}

/* 根据地址信息绑定和监听 */
static bool _bind_listen(struct comm_tcp *commtcp, struct addrinfo *ai)
{
	assert(commtcp && ai);

	bool            flag = false;
	struct addrinfo *aiptr = NULL;

	for (aiptr = ai; aiptr != NULL; aiptr = aiptr->ai_next) {
		commtcp->fd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);

		if (commtcp->fd <= 0) {
			continue;
		}

		/* 设置地址可重用 */
		int optval = 1;

		if (setsockopt(commtcp->fd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)sizeof(optval)) != 0) {
			CLOSEFD(commtcp->fd);	/* 发生错误，忽略此描述符，继续下一个描述符 */
			continue;
		}

		if (bind(commtcp->fd, aiptr->ai_addr, aiptr->ai_addrlen) != 0) {
			CLOSEFD(commtcp->fd);	/* 发生错误，忽略此描述符，继续下一个描述符 */
			continue;
		}

		/* 监听描述符 */
		if (unlikely(listen(commtcp->fd, MAX_LISTENFDS) == -1)) {
			CLOSEFD(commtcp->fd);	/* 发生错误，忽略此描述符，继续下一个描述符 */
			continue;
		}

		/* 将套接字设置为非阻塞模式 */
		if (unlikely(!fd_setopt(commtcp->fd, O_NONBLOCK))) {
			CLOSEFD(commtcp->fd);	/* 发生错误，忽略此描述符，继续下一个描述符 */
			continue;
		}

		flag = true;
		break;
	}

	return flag;
}

static bool _start_connect(struct comm_tcp *commtcp, struct addrinfo *ai, int cnt_timeout)
{
	assert(commtcp && ai);

	struct timeval timeout;

	/* 设置超时时间,timeout 小于 0，不设置超时
	 *			等于 0，设置默认值*/
	if (cnt_timeout == 0) {
		timeout.tv_sec = 0;			// 0秒
		timeout.tv_usec = 50000;		// 0.05秒
	} else {
		cnt_timeout = cnt_timeout * 1000;	// 转换成us
		timeout.tv_sec = cnt_timeout / 1000000;
		timeout.tv_usec = cnt_timeout % 1000000;
	}

	struct addrinfo *aiptr = NULL;

	for (aiptr = ai; aiptr != NULL; aiptr = aiptr->ai_next) {
		commtcp->fd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);

		if (commtcp->fd <= 0) {
			continue;
		}

		if (cnt_timeout >= 0) {
			if (setsockopt(commtcp->fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, (socklen_t)sizeof(struct timeval)) == -1) {
				perror("setsockopt()");
				CLOSEFD(commtcp->fd);	/* 发生其他的任何错误，都忽略此描述符，继续下一个 */
				continue;
			}
		}

		if (!fd_setopt(commtcp->fd, O_NONBLOCK)) {
			CLOSEFD(commtcp->fd);	/* 发生其他的任何错误，都忽略此描述符，继续下一个 */
			continue;
		}

		while (connect(commtcp->fd, aiptr->ai_addr, aiptr->ai_addrlen) == -1) {
			/* 连接失败 连接产生致命错误 */
			if ((errno != EINPROGRESS) && (errno != EALREADY)) {
				CLOSEFD(commtcp->fd);	/* 发生其他的任何错误，都忽略此描述符，继续下一个 */
				return false;
			}
		}

		loger(" connect return successed fd:%d\n", commtcp->fd);
		return true;
	}

	return false;
}

static bool _sconnect(struct comm_tcp *commtcp, struct addrinfo *aiptr)
{
	assert(commtcp && aiptr);

	fd_set          wset, rset;
	int             error = -1;
	int             retval = -1;
	int             len = sizeof(int);
	struct timeval  tm;

	if (connect(commtcp->fd, aiptr->ai_addr, aiptr->ai_addrlen) == 0) {
		loger("connect success on first try:%d\n", commtcp->fd);
		return true;
	}

	/* 连接产生致命错误 */
	if ((errno != EINPROGRESS) && (errno != EALREADY)) {
		loger("connect fatal error fd: %d errno:%d\n", commtcp->fd, errno);
		return false;
	}

	/* 连接正在进行中，检测是否会连接成功 */
	FD_ZERO(&wset);
	FD_ZERO(&rset);
	FD_SET(commtcp->fd, &wset);
	FD_SET(commtcp->fd, &rset);
	tm.tv_sec = TIMEOUTTIME;
	tm.tv_usec = 0;

	if (select(commtcp->fd + 1, &rset, &wset, NULL, &tm) < 1) {
		loger("select failed\n");
		return false;
	}

	if (FD_ISSET(commtcp->fd, &wset) || FD_ISSET(commtcp->fd, &rset)) {
		retval = getsockopt(commtcp->fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);

		if ((retval < 0) || error) {
			/*失败 Solaris版本将retval设为-1,Berkeley版本返回0,设置error错误值 */
			if ((error == EHOSTUNREACH) || (errno == EHOSTUNREACH)) {
				/* 错误值为此，则代表对端端口未打开 */
				// loger("%s\n", strerror(errno));
				loger("peer port isn't open\n");
			} else {
				loger("get socketopt error:%d fd:%d\n", error, commtcp->fd);
			}

			return false;
		} else {
			/* 连接成功 */
			loger("connect successed:%d\n", commtcp->fd);
			return true;
		}
	} else {
		loger("return select is other fd, not fd:%d\n", commtcp->fd);
		return false;
	}
}

int socket_send(struct comm_tcp *commtcp, char *data, size_t size)
{
	if (commtcp->stat == FD_CLOSE) {
		return -1;
	}

	commtcp->stat = FD_WRITE;

	int bytes = write(commtcp->fd, data, size);

	if (bytes > 0) {
		return bytes;
	} else {
		if (errno == EINTR) {
			/* 被打断，则下次继续处理 */
			return 0;
		} else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			/* 缓冲区已满 */
			return 0;
		} else {
			/* 出现其他的致命错误*/
			loger("write dead wrong\n");
			commtcp->stat = FD_CLOSE;
			return -1;
		}
	}
}

int socket_recv(struct comm_tcp *commtcp, char *data, size_t size)
{
	if (commtcp->stat == FD_CLOSE) {
		return -1;
	}

	commtcp->stat = FD_READ;

	int bytes = read(commtcp->fd, data, size);

	if (bytes == 0) {
		/* socket已经关闭 */
		commtcp->stat = FD_CLOSE;
		return -1;
	} else if (bytes < 0) {
		if (errno == EINTR) {
			/* 读操作被中断 */
			return 0;
		} else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			return 0;
		} else {
			/* 发生致命错误 */
			loger("read dead wrong\n");
			commtcp->stat = FD_CLOSE;
			return -1;
		}
	} else {
		return bytes;
	}
}

