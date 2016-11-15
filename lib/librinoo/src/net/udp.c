/**
 * @file   udp.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Oct 21 22:52:23 2013
 *
 * @brief  UDP socket management
 *
 *
 */

#include "rinoo/net/module.h"

extern const t_socket_class socket_class_udp;
extern const t_socket_class socket_class_udp6;

t_socket *rinoo_udp_client(t_sched *sched, t_ip *ip, uint16_t port)
{
	t_ip loopback;
	t_socket *socket;
	socklen_t addr_len;
	struct sockaddr *addr;

	if (ip == NULL) {
		memset(&loopback, 0, sizeof(loopback));
		loopback.v4.sin_family = AF_INET;
		loopback.v4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		ip = &loopback;
	}
	socket = rinoo_socket(sched, (IS_IPV6(ip) ? &socket_class_udp6 : &socket_class_udp));
	if (unlikely(socket == NULL)) {
		return NULL;
	}
	if (ip->v4.sin_family == AF_INET) {
		ip->v4.sin_port = htons(port);
		addr = (struct sockaddr *) &ip->v4;
		addr_len = sizeof(ip->v4);
	} else {
		ip->v6.sin6_port = htons(port);
		addr = (struct sockaddr *) &ip->v6;
		addr_len = sizeof(ip->v6);
	}
	if (rinoo_socket_connect(socket, addr, addr_len) != 0) {
		rinoo_socket_destroy(socket);
		return NULL;
	}
	return socket;
}
