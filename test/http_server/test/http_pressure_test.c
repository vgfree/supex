#define _XOPEN_SOURCE

// #include "../utils.h"

// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
#include <time.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char            *server = NULL;
short           port = 0;
unsigned long   client_thread = 0;
time_t          client_begin_tm = 0;

char    reconnect = 0;
char    long_post = 0;

char *g_short_post = {
	"POST /task/add.json HTTP/1.1\r\n"
	"Host: mirrtalk.com:9091\r\n"
	"Content-Type: application/json\r\n"
	"Content-Length: 0\r\n\r\n"
};

char *g_huge_post = {
	"POST /task/add.json HTTP/1.1\r\n"
	"Host: mirrtalk.com:9091\r\n"
	"Content-Type: application/json\r\n"
	"Content-Length: 1888\r\n\r\n"
	"MIIFbTCCBFWgAwIBAgICH4cwDQYJKoZIhvcNAQEFBQAwcDELMAkGA1UEBhMCVUsx"
	"ETAPBgNVBAoTCGVTY2llbmNlMRIwEAYDVQQLEwlBdXRob3JpdHkxCzAJBgNVBAMT"
	"AkNBMS0wKwYJKoZIhvcNAQkBFh5jYS1vcGVyYXRvckBncmlkLXN1cHBvcnQuYWMu"
	"dWswHhcNMDYwNzI3MTQxMzI4WhcNMDcwNzI3MTQxMzI4WjBbMQswCQYDVQQGEwJV"
	"SzERMA8GA1UEChMIZVNjaWVuY2UxEzARBgNVBAsTCk1hbmNoZXN0ZXIxCzAJBgNV"
	"BAcTmrsogriqMWLAk1DMRcwFQYDVQQDEw5taWNoYWVsIHBhcmQYJKoZIhvcNAQEB"
	"BQADggEPADCCAQoCggEBANPEQBgl1IaKdSS1TbhF3hEXSl72G9J+WC/1R64fAcEF"
	"W51rEyFYiIeZGx/BVzwXbeBoNUK41OK65sxGuflMo5gLflbwJtHBRIEKAfVVp3YR"
	"gW7cMA/s/XKgL1GEC7rQw8lIZT8RApukCGqOVHSi/F1SiFlPDxuDfmdiNzL31+sL"
	"0iwHDdNkGjy5pyBSB8Y79dsSJtCW/iaLB0/n8Sj7HgvvZJ7x0fr+RQjYOUUfrePP"
	"u2MSpFyf+9BbC/aXgaZuiCvSR+8Snv3xApQY+fULK/xY8h8Ua51iXoQ5jrgu2SqR"
	"wgA7BUi3G8LFzMBl8FRCDYGUDy7M6QaHXx1ZWIPWNKsCAwEAAaOCAiQwggIgMAwG"
	"A1UdEwEB/wQCMAAwEQYJYIZIAYb4QgHTTPAQDAgWgMA4GA1UdDwEB/wQEAwID6DAn"
	"BglghkgBhvhCAQ0EHxYdVUsgZS1TY2llbmNlIFVzZXIgQ2VydGlmaWNhdGUwHQYD"
	"VR0OBBYEFDTt/sf9PeMaZDHkUIldrDYMNTBZMIGaBgNVHSMEgZIwgY+AFAI4qxGj"
	"loCLDdMVKwiljjDastqooXSkcjBwMQswCQYDVQQGEwJVSzERMA8GA1UEChMIZVNj"
	"aWVuY2UxEjAQBgNVBAsTCUF1dGhvcml0eTELMAkGA1UEAxMCQ0ExLTArBgkqhkiG"
	"9w0BCQEWHmNhLW9wZXJhdG9yQGdyaWQtc3VwcG9ydC5hYy51a4IBADApBgNVHRIE"
	"IjAggR5jYS1vcGVyYXRvckBncmlkLXN1cHBvcnQuYWMudWswGQYDVR0gBBIwEDAO"
	"BgwrBgEEAdkvAQEBAQYwPQYJYIZIAYb4QgEEBDAWLmh0dHA6Ly9jYS5ncmlkLXN1"
	"cHBvcnQuYWMudmT4sopwqlBWsvcHViL2NybC9jYWNybC5jcmwwPQYJYIZIAYb4QgEDBDAWLmh0"
	"dHA6Ly9jYS5ncmlkLXN1cHBvcnQuYWMudWsvcHViL2NybC9jYWNybC5jcmwwPwYDrn"
	"VR0fBDgwNjA0oDKgMIYuaHR0cDovL2NhLmdyaWQt5hYy51ay9wdWIv"
	"Y3JsL2NhY3JsLmNybDANBgkqhkiG9w0BAQUFAAOCAQEAS/U4iiooBENGW/Hwmmd3"
	"XCy6Zrt08YjKCzGNjorT98g8uGsqYjSxv/hmi0qlnlHs+k/3Iobc3LjS5AMYr5L8"
	"UO7OSkgFFlLHQyC9JzPfmLCAugvzEbyv4Olnsr8hbxF1MbKZoQxUZtMVu29wjfXk"
	"hTeApBv7eaKCWpSp7MCbvgzm74izKhu3vlDk9w6qVrxePfGgpKPqfHiOoGhFnbTK"
	"wTC6o2xq5y0qZ03JonF7OJspEd3I5zKY3E+ov7/ZhW6DqT8UFvsAdjvQbXyhV8Eu"
	"Yhixw1aKEPzNjNowuIseVogKOLXxWI5vAi5HgXdS0/ES5gDGsABo4fqovUKlgop3"
	"RA=="
	"-----END CERTIFICATE-----"
};

void usage(void)
{
	printf("http_cli.out -s[server] -p[port] -l[1] -r[1] -c[request_count < 99999999] -t[clients num]\n");
	exit(1);
}

#define MAX_BUF_SIZE 2048

static unsigned long    send_fail_cnt = 0;
static unsigned long    recv_fail_cnt = 0;

extern int connect_server(const char *host, int client_port);

void *req_thread(void *arg)
{
	char buf[MAX_BUF_SIZE] = { 0 };

	int sock = 0;

	while (1) {
		sock = connect_server(server, port);

		if (sock > 0) {
			break;
		}
	}

	// printf("client react time: %ld\n", time(0) - client_begin_tm);

	unsigned long req_cnt = *(int *)arg;

	int     idx = 0;
	char    *body = NULL;
	int     type = 0;
	time_t  tm, valid_time;

	time_t  begin_tm = time(0);
	int     reconnect_cnt = 0;

	char    *post = NULL;
	int     len = 0;

	if (long_post) {
		len = strlen(g_huge_post);
		post = g_huge_post;
	} else {
		len = strlen(g_short_post);
		post = g_short_post;
	}

	for (;; ) {
		if (write(sock, post, len) != len) {
			send_fail_cnt++;
			close(sock);

			while (1) {
				sock = connect_server(server, port);

				if (sock > 0) {
					break;
				}
			}

			continue;
		}

		int nread = read(sock, buf, MAX_BUF_SIZE);

		if (nread <= 0) {
			recv_fail_cnt++;
			close(sock);	/* passive closed by server, client should close
					 *   socket itself */

			while (1) {
				sock = connect_server(server, port);

				if (sock > 0) {
					break;
				}
			}

			continue;
		}

		// printf("%d >>>> received %ld bytes:\n%s\n", idx, (long) nread, buf);

		idx++;

		if (reconnect && (idx % 7 == 0)) {
			close(sock);
			sock = connect_server(server, port);
			reconnect_cnt++;
		}

		if (idx == req_cnt) {
			printf("%ld request done, send_fail_cnt: %ld, recv_fail_cnt: %ld, time elapsed: %ld seconds\n",
				req_cnt, send_fail_cnt, recv_fail_cnt, (long)time(0) - begin_tm);
			char killer[] = { "POST /killServer.json HTTP/1.1\r\nHost:127.0.0.1:9091\r\nContent-Length:0\r\n\r\n" };
			write(sock, killer, strlen(killer));
			return NULL;
		}
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	unsigned long   req_cnt = 0;
	char            c = 0;

	while ((c = getopt(argc, argv, "s:p:r:l:c:t:h")) != -1) {
		switch (c)
		{
			case 's':
				server = optarg;
				break;

			case 'p':
				port = atoi(optarg);
				break;

			case 'r':
				reconnect = atoi(optarg);
				break;

			case 'l':
				long_post = atoi(optarg);
				break;

			case 'c':
				req_cnt = atol(optarg);

				if (req_cnt <= 0) {
					usage();
				}

				if (req_cnt > 99999999) {
					usage();
				}

				break;

			case 't':
				client_thread = atol(optarg);

				if (client_thread <= 0) {
					usage();
				}

				break;

			case 'h':
				usage();
				exit(EXIT_SUCCESS);

			default:
				printf("unrecognized option '%c'\n", c);
				usage();
				exit(EXIT_FAILURE);
		}
	}

	if (req_cnt == 0) {
		req_cnt = 100000;
	}

	if ((server == NULL) || (port == 0)) {
		usage();
	}

	if (client_thread == 0) {
		client_thread = 100;
	}

	int i;
	client_begin_tm = time(0);

	if (client_thread < 5) {
		for (i = 0; i < client_thread; ++i) {
			req_thread((void *)&req_cnt);
		}

		printf("used %ld seconds\n", (long)time(0) - client_begin_tm);
	} else {
		for (i = 0; i < client_thread; ++i) {
			pthread_t t;
			pthread_create(&t, NULL, req_thread, (void *)&req_cnt);
		}

		for (;; ) {
			sleep(1);
		}
	}

	return 0;
}

