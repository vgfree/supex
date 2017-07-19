/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_TCP_H__
#define __COMM_TCP_H__

#include <sys/select.h>
#include <sys/types.h>		/* See NOTES */
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include "comm_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAXIPADDRLEN 128	/* IP地址的最大长度 */

/* 端口的相关信息 */
struct comm_tcp
{
	int             fd;				/* 套接字描述符 */
	uint16_t        localport;			/* 本地端口 */
	uint16_t        peerport;			/* 对端端口 */
	char            localaddr[MAXIPADDRLEN];	/* 本地IP地址 */
	char            peeraddr[MAXIPADDRLEN];		/* 对端IP地址 */
	enum
	{
		COMM_CONNECT = 0x01,
		COMM_BIND,
		COMM_ACCEPT
	}               type;			/* 套接字的类型 */
	enum
	{
		FD_INIT = 0x01,			/*socket刚创建,未connect/bind/accept*/
		FD_READ,
		FD_WRITE,
		FD_CLOSE
	}               stat;			/* 套接字的状态 */
};

/* 设置socket套接字keep_alive选项 */
static inline bool set_keepalive(int fd)
{
	/*
	 * KeepAlive实现，单位秒
	 */
	int     keepAlive = 1;		// 设定KeepAlive
	int     keepIdle = 5;		// 开始首次KeepAlive探测前的TCP空闭时间
	int     keepInterval = 5;	// 两次KeepAlive探测间的时间间隔
	int     keepCount = 3;		// 判定断开前的KeepAlive探测次数

	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive)) == -1) {
		printf("setsockopt SO_KEEPALIVE error!\n");
		return false;
	}

	if (setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle)) == -1) {
		printf("(setsockopt TCP_KEEPIDLE error!\n");
		return false;
	}

	if (setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval)) == -1) {
		printf("setsockopt TCP_KEEPINTVL error!\n");
		return false;
	}

	if (setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount)) == -1) {
		printf("setsockopt TCP_KEEPCNT error!\n");
		return false;
	}

	return true;
}

/* 检查描述符是否可写 */
static inline bool check_writeable(int fd)
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
static inline bool check_readable(int fd)
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

/***********************************************************************************
* 功能: 绑定监听指定地址端口[地址端口填入到commtcp->localport和commtcp->localaddr]
* @返回值:true 监听成功 false 监听失败
***********************************************************************************/
bool socket_listen(struct comm_tcp *commtcp, const char *host, const char *port);

/***********************************************************************************
* 功能:连接到指定地址端口[地址端口填入到commtcp->peerlocal,commtcp->peeraddr]
* retry == -1为一直循环，直到成功。
* @返回值:true 连接成功 false 连接失败
***********************************************************************************/
bool socket_connect(struct comm_tcp *commtcp, const char *host, const char *port, int retry);

/***********************************************************************************
* 功能:接收一个新的连接
* @lsncommtcp:监听描述符相关信息
* @acptcommtcp:接收到新的描述符相关信息
* @返回值:< 0 接收失败 >0 新描述符的fd
***********************************************************************************/
int socket_accept(const struct comm_tcp *lsncommtcp, struct comm_tcp *acptcommtcp);

/*@return:
 *       0:重试
 *	-1:出错
 *	>0:成功
 */
int socket_send(struct comm_tcp *commtcp, char *data, size_t size);

/*@return:
 *       0:重试
 *	-1:出错
 *	>0:成功
 */
int socket_recv(struct comm_tcp *commtcp, char *data, size_t size);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_TCP_H__ */

