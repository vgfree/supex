/**
 * @file   socket_class_ssl.h
 * @author reginaldl <reginald.@gmail.com> - Copyright 2013
 * @date   Fri Mar  8 23:10:49 2013
 *
 * @brief  Secure socket class
 *
 *
 */

#ifndef RINOO_NET_SOCKET_CLASS_SSL_H_
#define RINOO_NET_SOCKET_CLASS_SSL_H_

t_socket *rinoo_socket_class_ssl_create(t_sched *sched);
void rinoo_socket_class_ssl_destroy(t_socket *socket);
ssize_t rinoo_socket_class_ssl_read(t_socket *socket, void *buf, size_t count);
ssize_t	rinoo_socket_class_ssl_write(t_socket *socket, const void *buf, size_t count);
int rinoo_socket_class_ssl_connect(t_socket *socket, const struct sockaddr *addr, socklen_t addrlen);
t_socket *rinoo_socket_class_ssl_accept(t_socket *socket, struct sockaddr *addr, socklen_t *addrlen);

#endif /* !RINOO_NET_SOCKET_CLASS_SSL_H_ */
