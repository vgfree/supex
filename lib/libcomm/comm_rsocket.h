/**
 * Provides a TCP socket connection to an address that is persistent. When there is a
 * disconnect or error, this transparently reconnects on the next socket operation
 * and keeps on going as though nothing went wrong. All sockets used are non-blocking.
 *
 * By its very nature, persistent sockets hide errors, so the functions don't
 * typically return errors. This is essentially a UDP socket that has guaranteed delivery
 * while connected.
 *
 * @file comm_rsocket.h
 * @author baoxue <huiqi.qian@sihua.com>
 * @copyright 2017
 */
/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_RSOCKET_H__
#define __COMM_RSOCKET_H__

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


/**
 * Everything behind the reconnecting socket
 */
typedef struct rsocket
{
	/**
	 * The underlying socket.
	 */
	int             sktfd;	/* 套接字描述符 */

	enum {
		RSOCKET_UNCONNECT = 0,
		RSOCKET_CONNECT,
	} cntstat;
	/**
	 * The address info that should be used to reconnect on disconnect.
	 */
	struct addrinfo *addrs;

	/**
	 * The current address that we're looking at in the address chain.
	 */
	struct addrinfo *curr_addr;
} rsocket_t;


int rsocket_open(struct rsocket *rsocket, const char *host, const char *port);

/**
 * Closes a rsocket connection.
 *
 * @param rsocket The reconnecting socket to close.
 */
void rsocket_close(struct rsocket *rsocket);

/**
 * Create a new connection.
 *
 * @param host The hostname to connect to.
 * @param port The port to connect to.
 * @param[out] rsocket Where the new reconnecting socket should be put.
 *
 * @return 0 on success.
 * @return -1 on address lookup error.
 */
/***********************************************************************************
* 功能:连接到指定地址端口[地址端口填入到rsocket->peerlocal,rsocket->peeraddr]
***********************************************************************************/
int rsocket_connect(struct rsocket *rsocket);

/***********************************************************************************
* 功能: 绑定监听指定地址端口[地址端口填入到rsocket->localport和rsocket->localaddr]
* @返回值:0 监听成功 -1 监听失败
***********************************************************************************/
int rsocket_bind_and_listen(struct rsocket *rsocket);


/***********************************************************************************
* 功能:接收一个新的连接
* @lstn_rsocket:监听描述符相关信息
* @acpt_rsocket:接收到新的描述符相关信息
* @返回值:< 0 接收失败 >0 新描述符的fd
***********************************************************************************/
int rsocket_accept(struct rsocket *rsocket);

/**
 * Send some data out to the server.
 *
 * @param rsocket The reconnecting socket
 * @param data The message to send
 * @param size The length of the message
 * @return:
 *       0:重试
 *	-1:出错
 *	>0:成功
 */
int rsocket_send(struct rsocket *rsocket, char *data, size_t size);

/**
 * Read data from the server.
 *
 * @param rsocket The socket to read from
 * @param data Where the data should be put
 * @param size The length of the data buffer
 * @return:
 *       0:重试
 *	-1:出错
 *	>0:成功
 */
int rsocket_recv(struct rsocket *rsocket, char *data, size_t size);




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

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_RSOCKET_H__ */
