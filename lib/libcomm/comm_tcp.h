/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_TCP_H__
#define __COMM_TCP_H__

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
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
		CONNECT_ONCE = 0x00,		/* 只连接服务器一次，失败就立刻返回 */
		CONNECT_ANYWAY = 0x1 << 4	/* 连接服务器失败，一直尝试连接，直到超时返回 */
	}               connattr;		/* COMM_CONNECT类型fd的连接属性 */
	enum
	{
		FD_INIT = 0x01,
		FD_READ,
		FD_WRITE,
		FD_CLOSE
	}               stat;			/* 套接字的状态 */
};

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
bool socket_listen(struct comm_tcp *commtcp, const char *host, const char *service);

/* 连接一个指定的地址端口号 @timeout[connattr为CONNECT_ANYWAY时才有效]:-1 一直阻塞到连接到对方 0 只连接一次失败理解 >0 超时立即返回 @connattr:CONNECT_ONCE 和CONNECT_ANYWAY */

/***********************************************************************************
* 功能:连接到指定地址端口[地址端口填入到commtcp->peerlocal,commtcp->peeraddr]
* @connattr:	CONNECT_ONCE   只连接一次无论成功与否都返回
*		CONNECT_ANYWAY 根据timeout的值确定尝试连接多少次
* @返回值:true 连接成功 false 连接失败
***********************************************************************************/
bool socket_connect(struct comm_tcp *commtcp, const char *host, const char *service, int connattr);


/***********************************************************************************
* 功能:接收一个新的连接
* @lsncommtcp:监听描述符相关信息
* @acptcommtcp:接收到新的描述符相关信息
* @返回值:< 0 接收失败 >0 新描述符的fd
***********************************************************************************/
int socket_accept(const struct comm_tcp *lsncommtcp, struct comm_tcp *acptcommtcp);


#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_TCP_H__ */

