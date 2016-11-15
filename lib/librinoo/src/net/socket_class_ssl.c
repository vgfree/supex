/**
 * @file   socket_class_ssl.c
 * @author reginaldl <reginald.l@gmail.com> - Copyright 2013
 * @date   Fri Mar  8 16:01:17 2013
 *
 * @brief  Secure socket class
 *
 *
 */

#include "rinoo/net/module.h"

const t_socket_class socket_class_ssl = {
	.domain = AF_INET,
	.type = SOCK_STREAM,
	.create = rinoo_socket_class_ssl_create,
	.destroy = rinoo_socket_class_ssl_destroy,
	.open = rinoo_socket_class_tcp_open,
	.dup = NULL,
	.close = rinoo_socket_class_tcp_close,
	.read = rinoo_socket_class_ssl_read,
	.recvfrom = NULL,
	.write = rinoo_socket_class_ssl_write,
	.writev = NULL,
	.sendto = NULL,
	.sendfile = NULL,
	.connect = rinoo_socket_class_ssl_connect,
	.bind = rinoo_socket_class_tcp_bind,
	.accept = rinoo_socket_class_ssl_accept
};

const t_socket_class socket_class_ssl6 = {
	.domain = AF_INET6,
	.type = SOCK_STREAM,
	.create = rinoo_socket_class_ssl_create,
	.destroy = rinoo_socket_class_ssl_destroy,
	.open = rinoo_socket_class_tcp_open,
	.dup = NULL,
	.close = rinoo_socket_class_tcp_close,
	.read = rinoo_socket_class_ssl_read,
	.recvfrom = NULL,
	.write = rinoo_socket_class_ssl_write,
	.writev = NULL,
	.sendto = NULL,
	.sendfile = NULL,
	.connect = rinoo_socket_class_ssl_connect,
	.bind = rinoo_socket_class_tcp_bind,
	.accept = rinoo_socket_class_ssl_accept
};

/**
 * Allocates a secure socket.
 *
 * @param sched Scheduler pointer
 *
 * @return Pointer to the new socket or NULL if an error occurs
 */
t_socket *rinoo_socket_class_ssl_create(t_sched *sched)
{
	t_ssl *ssl;

	ssl = calloc(1, sizeof(*ssl));
	if (unlikely(ssl == NULL)) {
		return NULL;
	}
	ssl->socket.node.sched = sched;
	return &ssl->socket;
}

/**
 * Frees a secure socket.
 *
 * @param socket Socket pointer
 */
void rinoo_socket_class_ssl_destroy(t_socket *socket)
{
	t_ssl *ssl = rinoo_ssl_get(socket);

	if (ssl->ssl != NULL) {
		SSL_free(ssl->ssl);
	}
	free(ssl);
}

/**
 * Replacement to the read(2) syscall in this library.
 * This function waits for the socket to be available for read operations and calls the read(2) syscall.
 *
 * @param socket Pointer to the socket to read
 * @param buf Buffer where to store the information read
 * @param count Buffer size
 *
 * @return The number of bytes read on success or -1 if an error occurs
 */
ssize_t rinoo_socket_class_ssl_read(t_socket *socket, void *buf, size_t count)
{
	int ret;
	t_ssl *ssl = rinoo_ssl_get(socket);

	if (rinoo_socket_waitio(socket) != 0) {
		return -1;
	}
	/* Don't need to wait for input here as SSL is buffered */
	while ((ret = SSL_read(ssl->ssl, buf, count)) < 0) {
		switch(SSL_get_error(ssl->ssl, ret)) {
		case SSL_ERROR_NONE:
			return 0;
		case SSL_ERROR_ZERO_RETURN:
		case SSL_ERROR_WANT_X509_LOOKUP:
		case SSL_ERROR_SYSCALL:
		case SSL_ERROR_SSL:
			return -1;
		case SSL_ERROR_WANT_READ:
			if (rinoo_socket_waitin(&ssl->socket) != 0) {
				return -1;
			}
			break;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_CONNECT:
		case SSL_ERROR_WANT_ACCEPT:
			if (rinoo_socket_waitout(&ssl->socket) != 0) {
				return -1;
			}
			break;

		}
	}
	if (ret <= 0) {
		return -1;
	}
	return ret;
}

/**
 * Replacement to the write(2) syscall in this library.
 * This function waits for the socket to be available for write operations and calls the write(2) syscall.
 *
 * @param socket Pointer to the socket to read
 * @param buf Buffer which stores the information to write
 * @param count Buffer size
 *
 * @return The number of bytes written on success or -1 if an error occurs
 */
ssize_t	rinoo_socket_class_ssl_write(t_socket *socket, const void *buf, size_t count)
{
	int ret;
	size_t sent;
	t_ssl *ssl = rinoo_ssl_get(socket);

	sent = count;
	while (count > 0) {
		if (rinoo_socket_waitio(socket) != 0) {
			return -1;
		}
		while ((ret = SSL_write(ssl->ssl, buf, count)) < 0) {
			switch(SSL_get_error(ssl->ssl, ret)) {
			case SSL_ERROR_NONE:
				return 0;
			case SSL_ERROR_ZERO_RETURN:
			case SSL_ERROR_WANT_X509_LOOKUP:
			case SSL_ERROR_SYSCALL:
			case SSL_ERROR_SSL:
				return -1;
			case SSL_ERROR_WANT_READ:
				if (rinoo_socket_waitin(socket) != 0) {
					return -1;
				}
				break;
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_CONNECT:
			case SSL_ERROR_WANT_ACCEPT:
				if (rinoo_socket_waitout(socket) != 0) {
					return -1;
				}
				break;
			}
		}
		if (ret <= 0) {
			return -1;
		}
		count -= ret;
		buf += ret;
	}
	return sent;
}

/**
 * Replacement to the connect(2) syscall.
 *
 * @param socket Pointer to the socket to connect.
 * @param addr Pointer to a sockaddr structure (see man connect)
 * @param addrlen Sockaddr structure size (see man connect)
 *
 * @return 0 on success or -1 if an error occurs (timeout is considered as an error)
 */
int rinoo_socket_class_ssl_connect(t_socket *socket, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret;
	BIO *sbio;
	t_ssl *ssl = rinoo_ssl_get(socket);

	if (unlikely(rinoo_socket_class_tcp_connect(socket, addr, addrlen) != 0)) {
		return -1;
	}
	ssl->ssl = SSL_new(ssl->ctx->ctx);
	if (unlikely(ssl->ssl == NULL)) {
		return -1;
	}
	sbio = BIO_new_socket(ssl->socket.node.fd, BIO_NOCLOSE);
	if (unlikely(sbio == NULL)) {
		return -1;
	}
	SSL_set_bio(ssl->ssl, sbio, sbio);
	while ((ret = SSL_connect(ssl->ssl)) < 0) {
		switch(SSL_get_error(ssl->ssl, ret)) {
		case SSL_ERROR_NONE:
			return 0;
		case SSL_ERROR_ZERO_RETURN:
		case SSL_ERROR_WANT_X509_LOOKUP:
		case SSL_ERROR_SYSCALL:
		case SSL_ERROR_SSL:
			return -1;
		case SSL_ERROR_WANT_READ:
			if (rinoo_socket_waitin(&ssl->socket) != 0) {
				return -1;
			}
			break;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_CONNECT:
		case SSL_ERROR_WANT_ACCEPT:
			if (rinoo_socket_waitout(&ssl->socket) != 0) {
				return -1;
			}
			break;

		}
	}
	if (ret == 0) {
		return -1;
	}
	return 0;
}

/**
 * Accepts a new connection from a listening socket.
 * This is a replacement to the accept(2) syscall in this library.
 *
 * @param socket Pointer to the socket which is listening to
 * @param addr Address to the peer socket (see man accept)
 * @param addrlen Sockaddr structure size (see man accept)
 *
 * @return A pointer to the new client socket or NULL if an error occurs
 */
t_socket *rinoo_socket_class_ssl_accept(t_socket *socket, struct sockaddr *addr, socklen_t *addrlen)
{
	int fd;
	int ret;
	BIO *sbio;
	t_ssl *new;
	t_ssl *ssl = rinoo_ssl_get(socket);

	errno = 0;
	while ((fd = accept4(ssl->socket.node.fd, addr, addrlen, SOCK_NONBLOCK)) < 0) {
		switch (errno) {
			case EAGAIN:
			case ENETDOWN:
			case EPROTO:
			case ENOPROTOOPT:
			case EHOSTDOWN:
			case ENONET:
			case EHOSTUNREACH:
			case EOPNOTSUPP:
			case ENETUNREACH:
				break;
			default:
				return NULL;
		}
		if (rinoo_socket_waitin(socket) != 0) {
			return NULL;
		}
		errno = 0;
	}
	new = calloc(1, sizeof(*new));
	if (unlikely(new == NULL)) {
		close(fd);
		return NULL;
	}
	new->ctx = ssl->ctx;
	new->socket.node.fd = fd;
	new->socket.node.sched = socket->node.sched;
	new->socket.class = socket->class;
	new->socket.parent = socket;
	new->ssl = SSL_new(new->ctx->ctx);
	if (unlikely(new->ssl == NULL)) {
		rinoo_socket_destroy(&new->socket);
		return NULL;
	}
	sbio = BIO_new_socket(new->socket.node.fd, BIO_NOCLOSE);
	if (unlikely(sbio == NULL)) {
		rinoo_socket_destroy(&new->socket);
		return NULL;
	}
	SSL_set_bio(new->ssl, sbio, sbio);
	while ((ret = SSL_accept(new->ssl)) <= 0) {
		switch(SSL_get_error(new->ssl, ret)) {
		case SSL_ERROR_NONE:
		case SSL_ERROR_ZERO_RETURN:
		case SSL_ERROR_WANT_X509_LOOKUP:
		case SSL_ERROR_SYSCALL:
		case SSL_ERROR_SSL:
			rinoo_socket_destroy(&new->socket);
			return NULL;
		case SSL_ERROR_WANT_READ:
			if (rinoo_socket_waitin(&new->socket) != 0) {
				rinoo_socket_destroy(&new->socket);
				return NULL;
			}
			break;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_CONNECT:
		case SSL_ERROR_WANT_ACCEPT:
			if (rinoo_socket_waitout(&new->socket) != 0) {
				rinoo_socket_destroy(&new->socket);
				return NULL;
			}
			break;

		}
	}
	return &new->socket;
}
