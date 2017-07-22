#pragma once

#include "comm_rsocket.h"

#define MAX_ADDR_LEN 128	/* IP地址的最大长度 */
#define MAX_PORT_LEN 128	/* IP地址的最大长度 */

struct comm_tcp
{
	char        	localport[MAX_PORT_LEN];			/* 本地端口 */
	char	 	   peerport[MAX_PORT_LEN];			/* 对端端口 */
	char            localaddr[MAX_ADDR_LEN];	/* 本地IP地址 */
	char            peeraddr[MAX_ADDR_LEN];		/* 对端IP地址 */
	enum
	{
		COMM_CONNECT = 0x01,
		COMM_BIND,
		COMM_ACCEPT
	}               type;			/* 套接字的类型 */

	struct rsocket rsocket;
};

void commtcp_set_portinfo(struct comm_tcp *commtcp, bool local, const char *host, const char *port);

bool commtcp_get_portinfo(struct comm_tcp *commtcp, bool local, char *host, char *port);
