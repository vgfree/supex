/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_tcp.h"

void commtcp_set_portinfo(struct comm_tcp *commtcp, bool local, const char *host, const char *port)
{
	if (local) {
		memcpy(commtcp->localport, port, strlen(port));
		memcpy(commtcp->localaddr, host, strlen(host));
	} else {
		memcpy(commtcp->peerport, port, strlen(port));
		memcpy(commtcp->peeraddr, host, strlen(host));
	}
}

/*获取端口信息 @local:true 为获取本地的IP地址和端口号， false：获取对端的IP地址和端口号 */
bool commtcp_get_portinfo(struct comm_tcp *commtcp, bool local, char *host, char *port)
{
	assert(commtcp && commtcp->rsocket.sktfd > 0);

	struct sockaddr sockaddr = {};
	socklen_t       len = sizeof(sockaddr);
	size_t          plen = sizeof(commtcp->localaddr);

	if (local) {
		if (getsockname(commtcp->rsocket.sktfd, &sockaddr, &len) != 0) {
			return false;
		}
	} else {
		if (getpeername(commtcp->rsocket.sktfd, &sockaddr, &len) != 0) {
			return false;
		}
	}

	const char      *pointer = NULL;
	switch (sockaddr.sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *inaddr = (struct sockaddr_in *)&sockaddr;
			pointer = inet_ntop(AF_INET, &inaddr->sin_addr, host, plen);
			sprintf(port, "%d", inaddr->sin_port);
		}
		break;

		case AF_INET6:
		{
			struct sockaddr_in6 *inaddr6 = (struct sockaddr_in6 *)&sockaddr;
			pointer = inet_ntop(AF_INET6, &inaddr6->sin6_addr, host, plen);
			sprintf(port, "%d", inaddr6->sin6_port);
		}
		break;

		case AF_UNIX:
		{
			struct sockaddr_un *unaddr = (struct sockaddr_un *)&sockaddr;
			snprintf(host, plen, "%s", unaddr->sun_path);
			pointer = host;
		}
		break;

		default:
			break;
	}

	return pointer != NULL ? true : false;
}
