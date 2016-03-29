#include "http.h"

#define STEP_GET_SIZE           1024
#define MAX_HEAD_SIZE           1024
#define MAX_HOST_SIZE           32
#define MAX_NAME_SIZE           32
#define MAX_VALUE_SIZE          128

#define HTTP_PORT               80
#define VALUE_HOST              "dev.mirrtalk.com"
#define VALUE_CONNECTION        "close"
#define VALUE_AGENT             "Mozilla/4.0"
#define VALUE_LANGUAGE          "zh-cn"
#define VALUE_TYPE              "application/json"
enum  index
{
	NAME_HOST = 0,
	NAME_CONNECTION,
	NAME_AGENT,
	NAME_LANGUAGE,
	NAME_LENGTH,
	NAME_TYPE
};
char head_tab[][MAX_NAME_SIZE] = {
	"Host:",
	"Connection:",
	"User-Agent:",
	"Accept-Language:",
	"Content-Length:",
	"Content-Type:"
};

static int add_http_head(char *buf, const char *name, const char *value)
{
	if (strlen(value) > MAX_VALUE_SIZE) {
		return -1;
	}

	strcat(buf, name);
	strcat(buf, value);
	strcat(buf, "\r\n");
	return 0;
}

int my_itoa(int n, char *string, int radix)
{
	int     i = 0;
	int     j;

	do {
		string[i] = n % radix + '0';
		i++;
	} while ((n /= radix) != 0);

	for (j = 0; j < i / 2; j++) {
		char t = string[j];
		string[j] = string[i - 1 - j];
		string[i - 1 - j] = t;
	}

	string[i] = '\0';

	return 1;
}

/*
 * url like this:  http://dev.mirrtalk.com/active
 * return: you shoud free when the packet is not to use.
 */
void *http_request(short method, char *url, ssize_t *back_size, void *data, size_t len)
{
	short   index = 0;
	char    host[MAX_HOST_SIZE] = { 0 };
	char    *req = NULL;
	char    *pkg_head = (char *)malloc(MAX_HEAD_SIZE);

	memset(pkg_head, 0, MAX_HEAD_SIZE);

	/*get host*/
	if (strncmp(url, "http://", strlen("http://"))) {
		req = url;
		strcat(host, VALUE_HOST);
	} else {
		req = url + strlen("http://");

		while ((*req != 0) && (*req != '/')) {
			host[index] = *req;
			req++;
			index++;
		}

		if (*req == 0) {
			return NULL;
		}
	}

	/*set url*/
	if (method == GET) {
		strcat(pkg_head, "GET ");
	} else {
		strcat(pkg_head, "POST ");
	}

	strcat(pkg_head, req);
	strcat(pkg_head, " HTTP/1.0\r\n");

	/* set http request head */
	/*you can design by yourself*/
	add_http_head(pkg_head, head_tab[NAME_HOST], host);
	add_http_head(pkg_head, head_tab[NAME_CONNECTION], VALUE_CONNECTION);
	add_http_head(pkg_head, head_tab[NAME_AGENT], VALUE_AGENT);
	add_http_head(pkg_head, head_tab[NAME_LANGUAGE], VALUE_LANGUAGE);

	if (method == POST) {
		char str_len[32] = { 0 };
		my_itoa(len, str_len, 10);
		add_http_head(pkg_head, head_tab[NAME_LENGTH], str_len);
		add_http_head(pkg_head, head_tab[NAME_TYPE], VALUE_TYPE);
	}

	/*over head*/
	strcat(pkg_head, "\r\n");

	printf("%s\n", pkg_head);		// just for debug,you shoud delete when online
	/*connect host*/
	int sockfd = tcp_connect(host, HTTP_PORT);
	/*send request*/
	tcp_send(sockfd, pkg_head, strlen(pkg_head));	/*send head*/

	if ((method == POST) && (data != NULL)) {
		printf("%s\n", (char *)data);	// just for debug,you shoud delete when online
		tcp_send(sockfd, data, len);	/*send body*/
	}

	/*get result*/
	void *r_bf = malloc(STEP_GET_SIZE);
	*back_size = 0;
	index = 1;

	while (1) {
		ssize_t get_size = tcp_receive(sockfd, r_bf, STEP_GET_SIZE);
		*back_size += get_size;

		if (get_size < STEP_GET_SIZE) {
			return r_bf;
		} else {
			index++;
			r_bf = realloc(r_bf, STEP_GET_SIZE * index);
		}
	}
}

