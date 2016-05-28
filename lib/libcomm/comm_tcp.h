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


#define MAXIPADDRLEN	128	/* IP地址的最大长度 */


/* 端口的相关信息 */
struct comm_tcp {
	int		fd;			/* 套接字描述符 */
	uint16_t	port;			/* 本地端口 */
	char		addr[MAXIPADDRLEN];	/* 本地IP地址 */
	enum {
		COMM_CONNECT = 0x01,
		COMM_BIND,
		COMM_ACCEPT
	}		type;			/* 套接字的类型*/
	enum {
		FD_INIT = 0x01,
		FD_READ,
		FD_WRITE,
		FD_CLOSE
	}		stat;			/* 套接字的状态 */
};


/* 检查描述符是否可写 */
static inline bool check_writeable(int fd)
{
	assert(fd > 0);
	fd_set wfds;
	FD_ZERO(&wfds);
	FD_SET(fd, &wfds);
	if (likely(select(fd+1, NULL, &wfds, NULL, NULL) > 0)) {
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
	if (likely(select(fd+1, &rfds, NULL, NULL, NULL) > 0)) {
		return true;
	} else {
		return false;
	}
}


/* 绑定监听一个指定地址端口号 */
bool socket_listen(struct comm_tcp *commtcp, const char *host, const char *service);

/* 连接一个指定的地址端口号 */
bool socket_connect(struct comm_tcp *commtcp, const char *host, const char *service);

/* 接收一个新的连接 @lsncommtcp:监听描述符相关信息 @acptcommtcp:接收新的描述符的相关信息 */
int socket_accept(const struct comm_tcp *lsncommtcp, struct comm_tcp *acptcommtcp);



#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_TCP_H__ */

