/**
 * @file   tcp.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Mar 20 18:10:17 2012
 *
 * @brief  Header file for TCP function declarations
 *
 *
 */

#ifndef RINOO_NET_TCP_H_
#define RINOO_NET_TCP_H_

#define RINOO_TCP_BACKLOG	128

t_socket *rinoo_tcp_client(t_sched *sched, t_ip *ip, uint16_t port, uint32_t timeout);
t_socket *rinoo_tcp_server(t_sched *sched, t_ip *ip, uint16_t port);
t_socket *rinoo_tcp_accept(t_socket *socket, t_ip *fromip, uint16_t *fromport);

#endif /* !RINOO_NET_TCP_H_ */
