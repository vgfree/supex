/**
 * @file   tcp.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Mar 20 18:06:41 2012
 *
 * @brief  TCP connection management
 *
 *
 */

#include "rinoo/net/module.h"

extern const t_socket_class socket_class_tcp;
extern const t_socket_class socket_class_tcp6;

/**
 * Creates a TCP client to be connected to a specific IP, on a specific port.
 *
 * @param sched Scheduler pointer
 * @param ip Destination IP to connect to
 * @param port Destination port to connect to
 * @param timeout Socket timeout
 *
 * @return Socket pointer on success or NULL if an error occurs
 */
t_socket *rinoo_tcp_client(t_sched *sched, t_ip *ip, uint16_t port, uint32_t timeout)
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
	socket = rinoo_socket(sched, (IS_IPV6(ip) ? &socket_class_tcp6 : &socket_class_tcp));
	if (unlikely(socket == NULL)) {
		return NULL;
	}
	if (timeout != 0 && rinoo_socket_timeout(socket, timeout) != 0) {
		rinoo_socket_destroy(socket);
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

/**
 * Creates a TCP server listening to a specific port, on a specific IP.
 *
 * @param sched Scheduler pointer
 * @param ip IP to bind
 * @param port Port to bind
 *
 * @return Socket pointer to the server on success or NULL if an error occurs
 */
t_socket *rinoo_tcp_server(t_sched *sched, t_ip *ip, uint16_t port)
{
	t_ip any;
	t_socket *socket;
	socklen_t addr_len;
	struct sockaddr *addr;

	if (ip == NULL) {
		memset(&any, 0, sizeof(any));
		any.v4.sin_family = AF_INET;
		any.v4.sin_addr.s_addr = INADDR_ANY;
		ip = &any;
	}
	socket = rinoo_socket(sched, (IS_IPV6(ip) ? &socket_class_tcp6 : &socket_class_tcp));
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
	if (rinoo_socket_bind(socket, addr, addr_len, RINOO_TCP_BACKLOG) != 0) {
		rinoo_socket_destroy(socket);
		return NULL;
	}
	return socket;
}

/**
 * Accepts a new connection from a listening socket.
 *
 * @param socket Pointer to the socket which is listening to
 * @param fromip Pointer to a t_ip where to store the from_ip
 * @param fromport Pointer to a uint32_t where to store the from_port
 *
 * @return A pointer to the new socket on success or NULL if an error occurs
 */
t_socket *rinoo_tcp_accept(t_socket *socket, t_ip *fromip, uint16_t *fromport)
{
	t_ip addr;
	socklen_t addr_len;
	t_socket *new;

	addr_len = sizeof(addr);
	new = rinoo_socket_accept(socket, (struct sockaddr *) &addr, &addr_len);
	if (fromip != NULL) {
		*fromip = addr;
	}
	if (fromport != NULL) {
		if (addr.v4.sin_family == AF_INET) {
			*fromport = ntohs(addr.v4.sin_port);
		} else {
			*fromport = ntohs(addr.v6.sin6_port);
		}
	}
	return new;
}
