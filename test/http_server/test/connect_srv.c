#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdio.h>

int connect_server(const char *host, int client_port)
{
	int sockfd = 0;

	unsigned long inaddr = inet_addr(host);

	if (inaddr != INADDR_NONE) {
		/* normal IP */
		struct sockaddr_in ad;
		memset(&ad, 0, sizeof(ad));
		ad.sin_family = AF_INET;

		memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
		ad.sin_port = htons(client_port);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if (sockfd < 0) {
			return -1;
		}

		if (connect(sockfd, (struct sockaddr *)&ad, sizeof(ad)) == 0) {
			return sockfd;
		}

		return -1;
	} else {
		char            port[6];/* 65535 */
		struct addrinfo hints, *res, *ressave;
		int             n = 0;

		snprintf(port, 6, "%d", client_port);
		memset(&hints, 0, sizeof(hints));

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		/* not IP, host name */
		if ((n = getaddrinfo(host, port, &hints, &res)) != 0) {
			return -1;
		}

		ressave = res;

		do {
			sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

			if (sockfd < 0) {
				continue;	/* ignore this one */
			}

			if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) {
				break;
			}

			close(sockfd);
		} while ((res = res->ai_next) != NULL);

		if (res == NULL) {
			return -1;

			freeaddrinfo(ressave);
		}

		freeaddrinfo(ressave);
		return sockfd;
	}

	return -1;
}

