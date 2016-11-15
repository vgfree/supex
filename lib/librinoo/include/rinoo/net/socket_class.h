/**
 * @file   socket_class.h
 * @author reginaldl <reginald.l@gmail.com> - Copyright 2013
 * @date   Fri Mar  8 15:25:01 2013
 *
 * @brief  Rinoo socket class
 *
 *
 */

#ifndef RINOO_NET_SOCKET_CLASS_H_
#define RINOO_NET_SOCKET_CLASS_H_

struct s_socket;

typedef struct s_socket_class {
	int domain;
	int type;
	struct s_socket *(*create)(t_sched *sched);
	void (*destroy)(struct s_socket *socket);
	int (*open)(struct s_socket *socket);
	struct s_socket *(*dup)(t_sched *destination, struct s_socket *socket);
	int (*close)(struct s_socket *socket);
	ssize_t (*read)(struct s_socket *socket, void *buf, size_t count);
	ssize_t (*recvfrom)(struct s_socket *socket, void *buf, size_t count, struct sockaddr *addrfrom, socklen_t *addrlen);
	ssize_t (*write)(struct s_socket *socket, const void *buf, size_t count);
	ssize_t (*writev)(struct s_socket *socket, t_buffer **buffers, int count);
	ssize_t (*sendto)(struct s_socket *socket, void *buf, size_t count, const struct sockaddr *addrto, socklen_t addrlen);
	ssize_t (*sendfile)(struct s_socket *socket, int in_fd, off_t offset, size_t count);
	int (*connect)(struct s_socket *socket, const struct sockaddr *addr, socklen_t addrlen);
	int (*bind)(struct s_socket *socket, const struct sockaddr *addr, socklen_t addrlen, int backlog);
	struct s_socket *(*accept)(struct s_socket *socket, struct sockaddr *addr, socklen_t *addrlen);
} t_socket_class;

#endif /* !RINOO_NET_SOCKET_CLASS_H_ */
