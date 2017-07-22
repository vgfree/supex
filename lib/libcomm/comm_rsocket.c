#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <poll.h>

#include "comm_rsocket.h"

#define  MAX_LISTENFDS  10240		/* 能够监听的描述符的个数 */

/* 获取地址信息 */
static bool _get_addrinfo(struct addrinfo **ai, const char *host, const char *port)
{
	assert(ai && host && port);

	struct addrinfo hints = {};
#if 1
	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
#else
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
#endif
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

int rsocket_open(struct rsocket *rsocket, const char *host, const char *port)
{
	assert(rsocket && host && port);
	memset(rsocket, 0, sizeof(*rsocket));

	struct timeval timeout;
	timeout.tv_sec = 0;			// 0秒
	timeout.tv_usec = 50000;		// 0.05秒

	int sock = -1;
	struct addrinfo *aires = NULL;
	struct addrinfo *aiptr = NULL;
	if (_get_addrinfo(&aires, host, port)) {
		for (aiptr = aires; aiptr; aiptr = aiptr->ai_next) {
			sock = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
			if (sock == -1) {
				continue;
			}

			if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, (socklen_t)sizeof(struct timeval)) == -1) {
				close(sock);
				sock = -1;
				continue;
			}
			/* 将套接字设置为非阻塞模式 */
			if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
				close(sock);
				sock = -1;
				continue;
			}
			break;
		}
	}

	if (sock == -1) {
		printf("socket failed!\n");
		if (aires) {
			freeaddrinfo(aires);
		}
		rsocket->sktfd = -1;
		rsocket->addrs = NULL;
		rsocket->curr_addr = NULL;
		rsocket->cntstat = RSOCKET_UNCONNECT;
		return -1;
	} else {
		rsocket->sktfd = sock;
		rsocket->addrs = aires;
		rsocket->curr_addr = aiptr;
		rsocket->cntstat = RSOCKET_UNCONNECT;
		return 0;
	}
}

/* 检查描述符是否可写 */
static bool check_writeable(int fd)
{
	assert(fd > 0);
	fd_set wfds;
	FD_ZERO(&wfds);
	FD_SET(fd, &wfds);

	if (likely(select(fd + 1, NULL, &wfds, NULL, NULL) > 0)) {
		return true;
	} else {
		return false;
	}
}

/* 检查描述符是否可读 */
static bool check_readable(int fd)
{
	assert(fd > 0);
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	if (likely(select(fd + 1, &rfds, NULL, NULL, NULL) > 0)) {
		return true;
	} else {
		return false;
	}
}

static int check_connect(int socketfd)
{
	struct pollfd fd;

	fd.fd = socketfd;
	fd.events = POLLOUT;

	while ( poll (&fd, 1, -1) == -1 ) {
		if( errno != EINTR ){
			perror("poll");
			return -1;
		}
	}

	int error = 0;
	socklen_t len = sizeof(error);
	/*失败 Solaris版本将retval设为-1,Berkeley版本返回0,设置error错误值 */
	if ( getsockopt (socketfd, SOL_SOCKET, SO_ERROR, &error, &len) == -1 ) {
		if (errno == EHOSTUNREACH) {
			/* 错误值为此，则代表对端端口未打开 */
			loger("peer port isn't open\n");
		}
		perror("getsockopt");
		return -1;
	}

	if(error != 0) {
		if (error == EHOSTUNREACH) {
			/* 错误值为此，则代表对端端口未打开 */
			loger("peer port isn't open\n");
		}
		fprintf (stderr, "socket %d connect failed: %s\n",
				socketfd, strerror (error));
		return -1;
	}

	return 0;
}

int rsocket_connect(struct rsocket *rsocket)
{
	int ret = 0;
	if (rsocket->cntstat == RSOCKET_CONNECT) {
		struct sockaddr addr = {.sa_family = AF_UNSPEC,};
		ret = connect(rsocket->sktfd, &addr, sizeof(addr));
		if (ret == -1) {
			if (errno != EINPROGRESS) {}
			return -1;
		}
		rsocket->cntstat = RSOCKET_UNCONNECT;
	}

	ret = connect(rsocket->sktfd, rsocket->curr_addr->ai_addr, rsocket->curr_addr->ai_addrlen);
	if (ret == -1) {
		if (errno != EISCONN) {
			if (errno == EINPROGRESS || errno == EALREADY || errno == EWOULDBLOCK) {
				ret = check_connect(rsocket->sktfd);
				if (ret == 0) {
					rsocket->cntstat = RSOCKET_CONNECT;
				}
				return ret;
			}
			return -1;
		}
	}

	rsocket->cntstat = RSOCKET_CONNECT;
	return 0;
}

/* 根据地址信息绑定和监听 */
int rsocket_bind_and_listen(struct rsocket *rsocket)
{
	/* 设置地址可重用 */
	int optval = 1;
	if (setsockopt(rsocket->sktfd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)sizeof(optval)) != 0) {
		return -1;
	}

	if (bind(rsocket->sktfd, rsocket->curr_addr->ai_addr, rsocket->curr_addr->ai_addrlen) != 0) {
		return -1;
	}

	/* 监听描述符 */
	if (unlikely(listen(rsocket->sktfd, MAX_LISTENFDS) == -1)) {
		return -1;
	}
	return 0;
}

int rsocket_accept(struct rsocket *rsocket)
{
	assert(rsocket->sktfd > 0);
	int sktfd = accept(rsocket->sktfd, NULL, NULL);

	if (sktfd <= 0) {
		return -1;
	}
	if (unlikely(!fd_setopt(sktfd, O_NONBLOCK))) {
		close(sktfd);
		return -1;
	}
	return sktfd;
}
#if 0
/**
 * Attempt to connect to again, looping through all the addresses available until
 * one works.
 */
#endif


void rsocket_close(struct rsocket *rsocket)
{
	close(rsocket->sktfd);
	if (rsocket->addrs) {
		freeaddrinfo(rsocket->addrs);
	}

	rsocket->sktfd = -1;
	rsocket->addrs = NULL;
	rsocket->curr_addr = NULL;
	rsocket->cntstat = RSOCKET_UNCONNECT;
}


int rsocket_send(struct rsocket *rsocket, char *data, size_t size)
{
	if (rsocket->cntstat == RSOCKET_UNCONNECT) {
		return -1;
	}

	int bytes = write(rsocket->sktfd, data, size);

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
			return -1;
		}
	}
}


int rsocket_recv(struct rsocket *rsocket, char *data, size_t size)
{
	if (rsocket->cntstat == RSOCKET_UNCONNECT) {
		return -1;
	}

	int bytes = read(rsocket->sktfd, data, size);

	if (bytes == 0) {
		/* socket已经关闭 */
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
			return -1;
		}
	} else {
		return bytes;
	}
}

