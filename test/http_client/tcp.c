#include "tcp.h"
#include <string.h>

int tcp_connect(char *ip, int port)
{
	struct sockaddr_in      servaddr;
	int                     sockfd = 0;

	/*create a socket*/
	sockfd = Socket(PF_INET, SOCK_STREAM, 0);

	/*set service info*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);

	/*connect*/
	Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	return sockfd;
}

ssize_t tcp_send(int fd, void *data, size_t len)
{
	return Write(fd, data, len);
}

ssize_t tcp_receive(int fd, void *buf, size_t bytes)
{
	return Read(fd, buf, bytes);
}

