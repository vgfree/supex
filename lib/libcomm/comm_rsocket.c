#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>

#include "rsocket.h"

struct rsocket
{
	/**
	 * The underlying socket.
	 */
	int             socket;

	enum {
		RSOCKET_UNCONNECT = 0,
		RSOCKET_CONNECT,
	} cntstat;
	/**
	 * The address info that should be used to reconnect on disconnect.
	 */
	struct addrinfo *addrs;

	/**
	 * The current address that we're looking at in the address chain.
	 */
	struct addrinfo *curr_addr;
};

int rsocket_open(const char *host, const int port, struct rsocket *rsocket)
{
	char cport[6];
	snprintf(cport, sizeof(cport), "%d", port);

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int sock = -1;
	struct addrinfo *res = NULL;
	struct addrinfo *info = NULL;
	if (getaddrinfo(host, cport, &hints, &res) == 0) {
		for (info = res; info; info = info->ai_next) {
			sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
			if (sock == -1) {
				continue;
			}

			if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
				close(sock);
				sock = -1;
				continue;
			}
			break;
		}
	}

	if (sock == -1) {
		printf("socket failed!\n");
		if (res) {
			freeaddrinfo(res);
		}
		rsocket->socket = -1;
		rsocket->addrs = NULL;
		rsocket->curr_addr = NULL;
		rsocket->cntstat = RSOCKET_UNCONNECT;
		return -1;
	} else {
		rsocket->socket = sock;
		rsocket->addrs = res;
		rsocket->curr_addr = info;
		rsocket->cntstat = RSOCKET_UNCONNECT;
		return 0;
	}
}

static int check_connect(int socketfd)
{
	struct pollfd fd;

	fd.fd = socketfd;
	fd.events = POLLOUT;

	while ( poll (&fd, 1, -1) == -1 ) {
		if( errno != EINTR ){
			perror("poll");
			return -1;
		}
	}

	int ret = 0;
	socklen_t len = sizeof(ret);
	if ( getsockopt (socketfd, SOL_SOCKET, SO_ERROR, &ret, &len) == -1 ) {
		perror("getsockopt");
		return -1;
	}

	if(ret != 0) {
		fprintf (stderr, "socket %d connect failed: %s\n",
				socketfd, strerror (ret));
		return -1;
	}

	return 0;
}

int rsocket_connect(struct rsocket *rsocket)
{
	int ret = 0;
	if (rsocket->cntstat == RSOCKET_CONNECT) {
		struct sockaddr addr = {.sa_family = AF_UNSPEC,};
		ret = connect(rsocket->socket, &addr, sizeof(addr));
		if (ret == -1) {
			if (errno != EINPROGRESS) {}
			return -1;
		}
		rsocket->cntstat = RSOCKET_UNCONNECT;
	}

	ret = connect(rsocket->socket, rsocket->curr_addr->ai_addr, rsocket->curr_addr->ai_addrlen);
	if (ret == -1) {
		if (errno == EINPROGRESS) {
			ret = check_connect(rsocket->socket);
			if (ret == 0) {
				rsocket->cntstat = RSOCKET_CONNECT;
			}
			return ret;
		}
		return -1;
	}

	rsocket->cntstat = RSOCKET_CONNECT;
	return 0;
}
#if 0
/**
 * Attempt to connect to again, looping through all the addresses available until
 * one works.
 */
#endif


void rsocket_close(struct rsocket *rsocket)
{
	close(rsocket->socket);
	freeaddrinfo(rsocket->addrs);

	rsocket->socket = -1;
	rsocket->addrs = NULL;
	rsocket->curr_addr = NULL;
	rsocket->cntstat = RSOCKET_UNCONNECT;
}


int rsocket_send(struct rsocket *rsocket, char *buff, int len)
{
	if (send(rsocket->socket, buff, len, MSG_NOSIGNAL) != len) {
		return -1;
	}
	return 0;
}

int rsocket_recv(struct rsocket *rsocket, char *buff, size_t len)
{
	int got = recv(rsocket->socket, buff, len, 0);

	if ((got <= 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
		return -1;
	}

	return got;
}

