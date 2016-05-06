/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_tcp.h"

#define  LISTENQ  1024	/* 能够监听的描述符的个数 */

int get_address(int fd, char *paddr, size_t plen)
{
	assert(paddr && plen);

	int		retval = -1;
	const char*	ptr = NULL;
	struct sockaddr sockaddr = {};
	socklen_t	len = sizeof(sockaddr);

	retval = getsockname(fd, &sockaddr, &len);
	if (unlikely(retval == -1)) {
		return retval;
	}

	switch (sockaddr.sa_family)
	{
		case AF_INET: {
				struct sockaddr_in *inaddr = (struct sockaddr_in*)&sockaddr;
				ptr = inet_ntop(AF_INET, &inaddr->sin_addr, paddr, plen);
			      }
				break;

		case AF_INET6: {
				struct sockaddr_in6 *inaddr6 = (struct sockaddr_in6*)&sockaddr;
				ptr = inet_ntop(AF_INET6, &inaddr6->sin6_addr, paddr, plen);
			       }
				break;

		case AF_UNIX: {
				struct sockaddr_un *unaddr = (struct sockaddr_un*)&sockaddr;
				snprintf(paddr, plen, "%s", unaddr->sun_path);
				ptr = paddr;
			      }
				break;

		default:
			break;
	}

	if (unlikely(ptr == NULL)) {
		return -1;;
	}

	return 0;
}

uint16_t get_port(int fd)
{
	int		retval = -1;
	uint16_t	port = 0;
	struct sockaddr sockaddr = {};
	socklen_t	len = sizeof(sockaddr);

	retval = getsockname(fd, &sockaddr, &len);
	if (unlikely(retval == -1)) {
		return retval;
	}

	switch (sockaddr.sa_family)
	{
		case AF_INET: {
				struct sockaddr_in *inaddr = (struct sockaddr_in*)&sockaddr;
				port = inaddr->sin_port;
			      }
				break;

		case AF_INET6: {
				struct sockaddr_in6 *inaddr6 = (struct sockaddr_in6*)&sockaddr;
				port = inaddr6->sin6_port;
			       }
				break;

		default:
			break;
	}

	return ntohs(port);
}

int socket_listen(const char* host, const char* service)
{
	assert(host && service);

	int			fd = -1;
	int			optval = 1;
	int			retval = -1;
	struct addrinfo*	aiptr = NULL;
	struct addrinfo*	ai = NULL;
	struct addrinfo		hints = {};

	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	while (1) {
		retval = getaddrinfo(host, service, &hints, &ai);
		if (likely(retval == 0)) {
			/* 函数成功返回 */
			break ;
		} else if (likely(retval == EAI_AGAIN)) {
			continue ;
		} else {
			return -1;
		}
	}
	for (aiptr = ai; aiptr != NULL; aiptr = aiptr->ai_next) {
		fd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
		if ( unlikely(fd < 0) ) {
			continue;
		}
		/* 允许地址的立即重用 */
		if (unlikely(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)sizeof(int)) == -1)) {
			close(fd);
			fd = -1;
			break;
		}

		retval = bind(fd, aiptr->ai_addr, aiptr->ai_addrlen);
		if (likely(retval == 0)) {
			/* 绑定成功 */
			if (unlikely(listen(fd, LISTENQ) == -1)) {
				close(fd);
				fd = -1;
				break;
			}
			/* 将套接字设置为非阻塞模式 */
			if (unlikely(!fd_setopt(fd, O_NONBLOCK))) {	
				close(fd);
				fd = -1;
			}
			break;
		}
		close(fd); /* 绑定失败，忽略此描述符 */
		fd = -1;
	}
	freeaddrinfo(ai);
	return fd;
}


int socket_connect(const char* host, const char* service)
{
	assert(host && service);

	int			fd = -1;
	int			retval = -1;
	struct addrinfo*	aiptr = NULL;
	struct addrinfo*	ai = NULL;
	struct addrinfo		hints = {};

	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	while (1) {
		retval = getaddrinfo(host, service, &hints, &ai);
		if (likely(retval == 0)) {
			break ;
		} else if (likely(retval == EAI_AGAIN)) {
			continue ;
		} else {
			return -1;
		}
	}
	for (aiptr = ai; aiptr != NULL; aiptr = aiptr->ai_next) {
		fd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
		if (unlikely(fd < 0)) {
			continue;
		}

		retval = connect(fd, aiptr->ai_addr, aiptr->ai_addrlen);
		if (likely(retval == 0)) {
			/* 设置套接字为非阻塞状态 */
			if (unlikely(!fd_setopt(fd, O_NONBLOCK))) {
				close(fd);
				fd = -1;
			}
			break;
		}
		close(fd);
		fd = -1;
	}
	freeaddrinfo(ai);
	return fd;
}
