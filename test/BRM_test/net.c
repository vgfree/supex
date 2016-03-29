#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "libmini.h"
#include "cache.h"
#include "net.h"

static struct addrinfo  *_get_tcpsocket_addrinfo(const char *host, const char *service, int flag, int family);

int tcp_listen(const char *host, const char *service)
{
	int                             sockfd = -1;
	struct addrinfo *volatile       addrinfo = NULL;

	TRY
	{
		int             ret = 0;
		struct addrinfo *temp = NULL;

		addrinfo = _get_tcpsocket_addrinfo(host, service, AI_PASSIVE, AF_UNSPEC);
		AssertRaise(addrinfo, EXCEPT_ASSERT);

		temp = addrinfo;

		for (temp = addrinfo; temp != NULL; temp = temp->ai_next) {
			sockfd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);

			if (unlikely(sockfd == -1)) {
				continue;
			}

			if (unlikely(!reuseaddr(sockfd))) {
				RAISE(EXCEPT_SYS);
			}

			ret = bind(sockfd, temp->ai_addr, temp->ai_addrlen);

			if (likely(ret == 0)) {
				ret = listen(sockfd, LISTENQ);
				AssertRaise(ret == 0, EXCEPT_SYS);
				break;
			}

			close(sockfd);
		}
	}
	CATCH
	{
		if (sockfd > 0) {
			close(sockfd);
			sockfd = -1;
		}
	}
	FINALLY
	{
		if (addrinfo) {
			freeaddrinfo(addrinfo);
			addrinfo = NULL;
		}
	}
	END;

	return sockfd;
}

bool set_sockfd(int sockfd, int flag)
{
	int ret = -1;

	ret = fcntl(sockfd, F_GETFL, 0);
	ret = fcntl(sockfd, F_SETFL, ret | flag);

	if (unlikely(ret == -1)) {
		x_printf(E, "set socket fd[%d] failed", sockfd);
		return false;
	}

	return true;
}

bool set_sockopt(int sockfd, int level, int optname, int optval)
{
	int rc = -1;

	rc = setsockopt(sockfd, level, optname, (void *)&optval, sizeof(optval));

	if (unlikely(rc == -1)) {
		x_printf(E, "set socket option failed");
		return false;
	}

	return true;
}

static struct addrinfo *_get_tcpsocket_addrinfo(const char *host, const char *service, int flag, int family)
{
	assert(host && service);

	struct  addrinfo *volatile addrinfo = NULL;

	TRY
	{
		int             n = 0;
		int             try = 0;
		int             trys = 10;
		struct addrinfo hints;

		while (1) {
			bzero(&hints, sizeof(hints));
			hints.ai_flags = flag | AI_CANONNAME;
			hints.ai_family = family;
			hints.ai_socktype = SOCK_STREAM;

			n = getaddrinfo(host, service, &hints, (struct addrinfo **)&addrinfo);

			if (unlikely(n != 0)) {
				if (n == EAI_AGAIN) {
					if (unlikely(try == trys)) {
						break;
					} else {
						try++;
						continue;
					}
				} else {
					x_printf(E, "%s", gai_strerror(n));
					break;
				}
			} else {
				break;
			}
		}

		AssertRaise(n == 0, EXCEPT_SYS);
	}
	CATCH
	{
		if (addrinfo) {
			x_printf(E, "get tcp address information failed");
			freeaddrinfo(addrinfo);
			addrinfo = NULL;
		}
	}
	END;

	return addrinfo;
}

