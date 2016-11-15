/**
 * @file   socket.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Dec 30 17:25:49 2009
 *
 * @brief  All functions needed to manage sockets.
 *
 *
 */

#include "rinoo/net/module.h"

/**
 * Socket initialisation function.
 * Initializes a socket depending on socket class.
 *
 * @param sched Pointer to a scheduler
 * @param sock Pointer to a socket structure to fill
 * @param class Socket class
 *
 * @return 0 on success, otherwise -1
 */
int rinoo_socket_init(t_sched *sched, t_socket *sock, const t_socket_class *class)
{
	XASSERT(sched != NULL, -1);
	XASSERT(sock != NULL, -1);
	XASSERT(class != NULL, -1);

	sock->class = class;
	sock->node.sched = sched;
	return class->open(sock);

}

/**
 * Socket creation function depending on socket class.
 *
 * @param sched Pointer to a scheduler
 * @param class Socket class
 *
 * @return A pointer to the new socket or NULL if an error occurs
 */
t_socket *rinoo_socket(t_sched *sched, const t_socket_class *class)
{
	t_socket *new;

	XASSERT(sched != NULL, NULL);
	XASSERT(class != NULL, NULL);

	new = class->create(sched);
	if (unlikely(new == NULL)) {
		return NULL;
	}
	new->class = class;
	if (unlikely(rinoo_socket_init(sched, new, class) != 0)) {
		class->destroy(new);
		return NULL;
	}
	return new;
}

/**
 * Socket dup function.
 *
 * @param destination Pointer to a scheduler
 * @param socket Socket to duplicate
 *
 * @return A pointer to the new socket or NULL if an error occurs
 */
t_socket *rinoo_socket_dup(t_sched *destination, t_socket *socket)
{
	t_socket *new;

	XASSERT(destination != NULL, NULL);
	XASSERT(socket != NULL, NULL);

	new = socket->class->dup(destination, socket);
	if (unlikely(new == NULL)) {
		return NULL;
	}
	new->node.sched = destination;
	return new;
}

/**
 * Close a socket only (does not free memory).
 *
 * @param socket Pointer to the socket to close
 */
void rinoo_socket_close(t_socket *socket)
{
	XASSERTN(socket != NULL);

	rinoo_sched_remove(&socket->node);
	socket->class->close(socket);
	memset(&socket->node, 0, sizeof(socket->node));
}

/**
 * Destroys a socket.
 *
 * @param socket Pointer to the socket to destroy
 */
void rinoo_socket_destroy(t_socket *socket)
{
	XASSERTN(socket != NULL);

	rinoo_socket_close(socket);
	socket->class->destroy(socket);
}

/**
 * Releases socket execution and waits for the socket to be available for read operations.
 *
 * @param socket Pointer to the socket to wait for
 *
 * @return 0 on success or -1 if an error occurs (timeout is considered as error)
 */
int rinoo_socket_waitin(t_socket *socket)
{
	socket->io_calls = 0;
	return rinoo_sched_waitfor(&socket->node, RINOO_MODE_IN);
}

/**
 * Releases socket execution and waits for the socket to be available for write operations.
 *
 * @param socket Pointer to the socket to wait for
 *
 * @return 0 on success or -1 if an error occurs (timeout is considered as error)
 */
int rinoo_socket_waitout(t_socket *socket)
{
	socket->io_calls = 0;
	return rinoo_sched_waitfor(&socket->node, RINOO_MODE_OUT);
}

/**
 * Increments internal io counter and releases the socket if too many io operations
 * have been done consecutively.
 *
 * @param socket Pointer to the socket to wait for
 *
 * @return 0 on success or -1 if an error occurs
 */
int rinoo_socket_waitio(t_socket *socket)
{
	socket->io_calls++;
	if (socket->io_calls > MAX_IO_CALLS) {
		socket->io_calls = 0;
		if (rinoo_task_pause(socket->node.sched) != 0) {
			return -1;
		}
	}
	return 0;
}

/**
 * Schedules a socket to be waken up.
 *
 * @param socket Socket pointer
 * @param ms Timeout in milliseconds
 *
 * @return 0 on success or -1 if an error occurs
 */
int rinoo_socket_timeout(t_socket *socket, uint32_t ms)
{
	struct timeval res;
	struct timeval toadd;

	XASSERT(socket != NULL, -1);

	if (ms == 0) {
		return rinoo_task_schedule(rinoo_task_driver_getcurrent(socket->node.sched), NULL);
	}
	toadd.tv_sec = ms / 1000;
	toadd.tv_usec = (ms % 1000) * 1000;
	timeradd(&socket->node.sched->clock, &toadd, &res);
	return rinoo_task_schedule(rinoo_task_driver_getcurrent(socket->node.sched), &res);
}

/**
 * Connects a socket if possible by socket class.
 *
 * @param socket Pointer to the socket to connect
 * @param addr Pointer to a sockaddr structure (see man connect)
 * @param addrlen Sockaddr structure size (see man connect)
 *
 * @return 0 on success or -1 if an error occurs (timeout is considered as an error)
 */
int rinoo_socket_connect(t_socket *socket, const struct sockaddr *addr, socklen_t addrlen)
{
	XASSERT(socket != NULL, -1);
	XASSERT(socket->class->connect != NULL, -1);

	return socket->class->connect(socket, addr, addrlen);
}

/**
 * Marks the specified socket to be listening to new connection using socket class.
 *
 * @param socket Pointer to the socket to listen to
 * @param addr Pointer to a sockaddr structure (see man listen)
 * @param addrlen Sockaddr structure size (see man listen)
 * @param backlog Maximum listening queue size (see man listen)
 *
 * @return 0 on success or -1 if an error occurs
 */
int rinoo_socket_bind(t_socket *socket, const struct sockaddr *addr, socklen_t addrlen, int backlog)
{
	XASSERT(socket != NULL, -1);
	XASSERT(socket->class->bind != NULL, -1);

	return socket->class->bind(socket, addr, addrlen, backlog);
}

/**
 * Accepts a new connection from a listening socket.
 *
 * @param socket Pointer to the socket which is listening to
 * @param addr Address to the peer socket (see man accept)
 * @param addrlen Sockaddr structure size (see man accept)
 *
 * @return A pointer to the new client socket or NULL if an error occurs
 */
t_socket *rinoo_socket_accept(t_socket *socket, struct sockaddr *addr, socklen_t *addrlen)
{
	XASSERT(socket != NULL, NULL);
	XASSERT(socket->class->accept != NULL, NULL);

	return socket->class->accept(socket, addr, addrlen);
}

/**
 * Calls the appropriate read function depending on socket class.
 *
 * @param socket Pointer to the socket to read
 * @param buf Buffer where to store the information read
 * @param count Buffer size
 *
 * @return The number of bytes read on success or -1 if an error occurs
 */
ssize_t rinoo_socket_read(t_socket *socket, void *buf, size_t count)
{
	return socket->class->read(socket, buf, count);
}

/**
 * Calls the appropriate recvfrom function depending on socket class.
 *
 * @param socket Pointer to the socket to read
 * @param buf Buffer where to store the information read
 * @param count Buffer size
 * @param addrfrom Sockaddr where to store the source address
 * @param addrlen Sockaddr length
 *
 * @return The number of bytes read on success or -1 if an error occurs
 */
ssize_t rinoo_socket_recvfrom(t_socket *socket, void *buf, size_t count, struct sockaddr *addrfrom, socklen_t *addrlen)
{
	XASSERT(socket->class->recvfrom != NULL, -1);

	return socket->class->recvfrom(socket, buf, count, addrfrom, addrlen);
}

/**
 * Calls the appropriate function depending on socket class.
 *
 * @param socket Pointer to the socket to read
 * @param buf Buffer which stores the information to write
 * @param count Buffer size
 *
 * @return The number of bytes written on success or -1 if an error occurs
 */
ssize_t	rinoo_socket_write(t_socket *socket, const void *buf, size_t count)
{
	return socket->class->write(socket, buf, count);
}

/**
 * Calls the appropriate function depending on socket class.
 *
 * @param socket Pointer to the socket to read
 * @param buffers Array of buffers
 * @param count Array size
 *
 * @return The number of bytes written on success or -1 if an error occurs
 */
ssize_t	rinoo_socket_writev(t_socket *socket, t_buffer **buffers, int count)
{
	int i;
	ssize_t ret;
	ssize_t total;

	if (socket->class->writev != NULL) {
		return socket->class->writev(socket, buffers, count);
	} else {
		total = 0;
		for (i = 0; i < count; i++) {
			ret = rinoo_socket_writeb(socket, buffers[i]);
			if (ret < 0) {
				return -1;
			}
			total += ret;
		}
		return total;
	}
}

/**
 * Calls the appropriate sendto function depending on socket class.
 *
 * @param socket Pointer to the socket to read
 * @param buf Buffer which stores the information to write
 * @param count Buffer size
 * @param addrto Address to send to
 * @param addrlen Sockaddr length
 *
 * @return The number of bytes written on success or -1 if an error occurs
 */
ssize_t rinoo_socket_sendto(t_socket *socket, void *buf, size_t count, const struct sockaddr *addrto, socklen_t addrlen)
{
	XASSERT(socket->class->sendto != NULL, -1);

	return socket->class->sendto(socket, buf, count, addrto, addrlen);
}

/**
 * Socket read interface for t_buffer.
 * This function waits for and reads information available on the socket.
 * Adds the result to the t_buffer structure. It does extend the buffer if necessary.
 *
 * @param socket Pointer to the socket to read
 * @param buffer Buffer where to store the result
 *
 * @return The number of bytes read on success or -1 if an error occurs
 */
ssize_t rinoo_socket_readb(t_socket *socket, t_buffer *buffer)
{
	ssize_t res;

	if (buffer_isfull(buffer) && buffer_extend(buffer, buffer_size(buffer)) != 0) {
		return -1;
	}
	res = socket->class->read(socket,
				  buffer_ptr(buffer) + buffer_size(buffer),
				  buffer_msize(buffer) - buffer_size(buffer));
	if (res <= 0) {
		return -1;
	}
	buffer_setsize(buffer, buffer_size(buffer) + res);
	return res;
}

/**
 * Reads a line from a socket.
 * This function waits for and reads information available on a socket until
 * it finds a delimiter or the maximum line size speficied.
 * Adds the result to the t_buffer structure. It does extend the buffer if necessary.
 * The size returned is the size of the last line found. It is actually not the total
 * amount of bytes read. The function can implicitly store more data in buffer.
 * If the calling function aims to call sequentially rinoo_socket_readline
 * with the same buffer, it has to remove the last line from it by calling buffer_erase
 * with the returned size.
 *
 * @param socket Pointer to the socket to read
 * @param buffer Pointer to the buffer where to store data read
 * @param delim Pointer to the line delimiter
 * @param maxsize Maximum line size to be read
 *
 * @return The size of the last line found, or -1 if an error occurs.
 */
ssize_t rinoo_socket_readline(t_socket *socket, t_buffer *buffer, const char *delim, size_t maxsize)
{
	void *ptr;
	size_t dlen;
	ssize_t res;
	off_t offset;

	offset = 0;
	dlen = strlen(delim);
	while (buffer_size(buffer) < maxsize) {
		if (buffer_size(buffer) - offset >= dlen) {
			ptr = memmem(buffer_ptr(buffer) + offset, buffer_size(buffer) - offset, delim, dlen);
			if (ptr != NULL) {
				return (ptr - buffer_ptr(buffer) + dlen);
			}
			offset = buffer_size(buffer) - dlen;
		}
		if (buffer_isfull(buffer) && buffer_extend(buffer, buffer_size(buffer)) != 0) {
			return -1;
		}
		res = socket->class->read(socket,
					  buffer_ptr(buffer) + buffer_size(buffer),
					  buffer_msize(buffer) - buffer_size(buffer));
		if (res <= 0) {
			return -1;
		}
		buffer_setsize(buffer, buffer_size(buffer) + res);
	}
	return maxsize;
}

/**
 * This function reads socket input to find an expected string.
 *
 * @param socket Pointer to the socket to read
 * @param buffer Pointer to the buffer where to store data read
 * @param expected Expected string to be matched
 *
 * @return Buffer size if the string has been found, otherwise -1
 */
ssize_t rinoo_socket_expect(t_socket *socket, t_buffer *buffer, const char *expected)
{
	size_t len;

	len = strlen(expected);
	while (buffer_size(buffer) < len) {
		if (rinoo_socket_readb(socket, buffer) <= 0) {
			return -1;
		}
	}
	if (buffer_strncmp(buffer, expected, len) == 0) {
		return buffer_size(buffer);
	}
	return -1;
}

/**
 * Socket write interface for t_buffer.
 * This function waits for the socket to be available for write operations and writes the buffer content into the socket.
 *
 * @param socket Pointer to the socket to write to
 * @param buffer Buffer which contains the information to write
 *
 * @return The number of bytes written on success or -1 if an error occurs
 */
ssize_t rinoo_socket_writeb(t_socket *socket, t_buffer *buffer)
{
	size_t len;
	size_t total;
	ssize_t res;

	total = 0;
	len = buffer_size(buffer);
	while (len > 0) {
		res = socket->class->write(socket, buffer_ptr(buffer) + buffer_size(buffer) - len, len);
		if (res <= 0) {
			return -1;
		}
		total += res;
		len -= res;
	}
	return total;
}

/**
 * Send a file through a socket.
 * This function waits for the socket to be available for write operations and attempt to send file content.
 *
 * @param socket Pointer to the socket to write to
 * @param in_fd File descriptor of the file to send
 * @param offset File offset
 * @param count Number of bytes to send
 *
 * @return Number of bytes sent or -1 if an error occurs
 */
ssize_t rinoo_socket_sendfile(t_socket *socket, int in_fd, off_t offset, size_t count)
{
	if (unlikely(socket->class->sendfile == NULL)) {
		void *ptr;
		int pagesize;
		ssize_t result;
		t_buffer dummy;

		pagesize = getpagesize();
		ptr = mmap(NULL, count + (offset % pagesize), PROT_READ, MAP_PRIVATE, in_fd, pagesize * (offset / pagesize));
		if (ptr == MAP_FAILED) {
			return -1;
		}
		buffer_static(&dummy, ptr + (offset % pagesize), count);
		result = rinoo_socket_writeb(socket, &dummy);
		munmap(ptr, count + (offset % pagesize));
		return result;
	}
	return socket->class->sendfile(socket, in_fd, offset, count);
}
