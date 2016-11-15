/**
 * @file   udp.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Oct 21 23:27:26 2013
 *
 * @brief  Header file for UDP function declarations
 *
 *
 */

#ifndef RINOO_NET_UDP_H_
#define RINOO_NET_UDP_H_

t_socket *rinoo_udp_client(t_sched *sched, t_ip *ip, uint16_t port);

#endif /* !RINOO_NET_UDP_H_ */
