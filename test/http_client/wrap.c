#include "wrap.h"

void perr_exit(const char *s)
{
	perror(s);
	exit(1);
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int n;

again:

	if ((n = accept(fd, sa, salenptr)) < 0) {
		if ((errno == ECONNABORTED) || (errno == EINTR)) {
			goto again;
		} else {
			perr_exit("accept error");
		}
	}

	return n;
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0) {
		perr_exit("bind error");
	}
}

void Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (connect(fd, sa, salen) < 0) {
		perr_exit("connect error");
	}
}

void Listen(int fd, int backlog)
{
	if (listen(fd, backlog) < 0) {
		perr_exit("listen error");
	}
}

int Socket(int family, int type, int protocol)
{
	int n;

	if ((n = socket(family, type, protocol)) < 0) {
		perr_exit("socket error");
	}

	return n;
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t n;

again:

	if ((n = read(fd, ptr, nbytes)) == -1) {
		if (errno == EINTR) {
			goto again;
		} else {
			return -1;
		}
	}

	return n;
}

ssize_t Write(int fd, const void *ptr, size_t nbytes)
{
	ssize_t n;

again:

	if ((n = write(fd, ptr, nbytes)) == -1) {
		if (errno == EINTR) {
			goto again;
		} else {
			return -1;
		}
	}

	return n;
}

void Close(int fd)
{
	if (close(fd) == -1) {
		perr_exit("close error");
	}
}

ssize_t Readn(int fd, void *vptr, size_t n)
{
	size_t  nleft;
	ssize_t nread;
	char    *ptr;

	ptr = vptr;
	nleft = n;

	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR) {
				nread = 0;
			} else {
				return -1;
			}
		} else if (nread == 0) {
			break;
		}

		nleft -= nread;
		ptr += nread;
	}

	return n - nleft;
}

ssize_t Writen(int fd, const void *vptr, size_t n)
{
	size_t          nleft;
	ssize_t         nwritten;
	const char      *ptr;

	ptr = vptr;
	nleft = n;

	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			if ((nwritten < 0) && (errno == EINTR)) {
				nwritten = 0;
			} else {
				return -1;
			}
		}

		nleft -= nwritten;
		ptr += nwritten;
	}

	return n;
}

