/*
 * test receive after server closed the client socket
 * auth: coanor <coanor at gmail dot com>
 * date: Tue Aug 27 14:47:18 CST 2013
 */
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define KNRM            "\x1B[0m"
#define KRED            "\x1B[31m"
#define KGRN            "\x1B[32m"
#define KYEL            "\x1B[33m"
#define KBLU            "\x1B[34m"
#define KMAG            "\x1B[35m"
#define KCYN            "\x1B[36m"
#define KWHT            "\x1B[37m"

#define MAX_BUF_SIZE    4096

extern int connect_server(const char *host, int client_port);

int main(int argc, char *argv[])
{
	const char      *host;
	short           port;
	int             sleep_time;
	int             block;

	char c;

	while ((c = getopt(argc, argv, "s:p:t:b:")) != -1) {
		switch (c)
		{
			case 's':
				host = optarg; break;

			case 'p':
				port = atoi(optarg); break;

			case 't':
				sleep_time = atoi(optarg); break;

			case 'b':
				block = atoi(optarg); break;

			default:
				printf("%sunrecognized option '%c'\n", KRED, c);
				exit(1);
		}
	}

	int sock = connect_server(host, port);

	if (sock < 0) {
		printf("%sconnect host(%s:%d) fail.\n", KRED, host, port);
		exit(1);
	}

	char req[] = "POST /task/add.json HTTP/1.1\r\n"
		"Host: mirrtalk.com:9091\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0\r\n\r\n";

	size_t len = sizeof(req) - 1;

	if (write(sock, req, len) != len) {
		printf("%swrite failed.\n", KRED);
		close(sock);
		exit(1);
	} else {
		/* lazy receive */
		char buf[MAX_BUF_SIZE] = { 0 };

		if (sleep_time > 0) {
			printf("%ssleeping(%ds)...\n", KYEL, sleep_time);
			sleep(sleep_time);
		}

		if (block) {
			/* set nonblock */
			printf("%stry nonblock receiving...\n", KYEL);
			fcntl(sock, F_SETFL, O_NONBLOCK);
		} else {
			printf("%stry block receiving...\n", KYEL);
		}

		while (1) {
			int nread = read(sock, buf, MAX_BUF_SIZE);

			if (nread == 0) {
				printf("%sserver closed, receive error.\n", KRED);
				close(sock);
				exit(1);
			} else if (nread < 0) {
				printf("%sreceive done.\n", KGRN);
				break;
			} else {
				printf("%sreceived %d bytes.\n", KMAG, nread);
				// printf("%s%s\n", KGRN, buf);
			}
		}
	}

	printf("%sexiting...\n", KGRN);
	close(sock);

	return 0;
}

