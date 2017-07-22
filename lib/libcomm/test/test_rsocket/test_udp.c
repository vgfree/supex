#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SERV_PORT 12345

void print_local_addr(int s)
{
	struct sockaddr_in localaddr;
	socklen_t len = sizeof(localaddr);

	if (getsockname(s, (struct sockaddr *)&localaddr, &len) != 0) {
		perror("getsockname failed");
	}

	char temp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &localaddr.sin_addr, temp, INET_ADDRSTRLEN);

	printf("Local binding: address=%s, port=%d\n", temp, ntohs(localaddr.sin_port));
}


int main(int argc, char** argv)
{
	if (argc != 2) {
		printf("Usage: disconnect_udp <ipaddress>");
		exit(0);
	}
	char *ipaddress = argv[1];

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	// creat a UDP socket which binds to a local address
	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, ipaddress, &srv_addr.sin_addr) != 1) {
		perror("inet_pton failed");
	}
	bind(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
	print_local_addr(sockfd);

	// connect this UDP socket
	struct sockaddr_in cli_addr;
	bzero(&cli_addr, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(SERV_PORT);
	if (inet_pton(AF_INET, ipaddress, &cli_addr.sin_addr) != 1) {
		perror("inet_pton failed");
	}
	if (connect(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) != 0) {
		perror("connect failed");
	}
	print_local_addr(sockfd);

	// disconnect it
	cli_addr.sin_family = AF_UNSPEC;
	if (connect(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) != 0) {
		perror("connect failed");
	}
	print_local_addr(sockfd);

	close(sockfd);
}

