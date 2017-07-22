#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "rsocket.c"

#define HOST    "localhost"
#define PORT    4444

#define TEST(expr)						 \
	total++;						 \
	if (!(expr)) {						 \
		failed++;					 \
		printf("Fail (line %d): %s\n", __LINE__, #expr); \
		return 1;					 \
	}

#define ACCEPT(client)				     \
	{					     \
		client = accept(server, NULL, NULL); \
		TEST(client != -1);		     \
		fcntl(client, F_SETFL, O_NONBLOCK);  \
	}

/**
 * The signature for all test functions.
 *
 * @param rsocket The socket that was opened to the server.
 */
typedef int (*test_fn)(struct rsocket *rsocket);

/**
 * The socket the server is listening on.
 */
static int server = 0;

/**
 * Counters for tests.
 */
static unsigned int     total = 0;
static unsigned int     failed = 0;
static unsigned int     total_tests = 0;
static unsigned int     failed_tests = 0;

int test_normal(struct rsocket *rsocket)
{
	// Just make sure a connection comes through
	int client = 0;
	ACCEPT(client);

	/*c --> s*/
	rsocket_send(rsocket, "test", 4);

	char buff[5];
	memset(buff, 0, sizeof(buff));
	TEST(read(client, buff, sizeof(buff)) == 4);
	TEST(strcmp(buff, "test") == 0);


	/*s --> c*/
	send(client, "test", 4, MSG_NOSIGNAL);

	memset(buff, 0, sizeof(buff));
	TEST(rsocket_recv(rsocket, buff, sizeof(buff) - 1) == 4);
	TEST(strcmp(buff, "test") == 0);


	if (client) {
		close(client);
	}
	return 0;
}


int test_send_reconnect(struct rsocket *rsocket)
{
	int client = 0;
	ACCEPT(client);
	if (client) {
		close(client);
	}

	// Make sure the other side knows it's closed
	while (1) {
		int err = rsocket_send(rsocket, "test", 4);
		if (err) {
			rsocket_connect(rsocket);
			break;
		}
	}

	ACCEPT(client);

	rsocket_send(rsocket, "test", 4);

	char buff[5];
	memset(buff, 0, sizeof(buff));
	TEST(read(client, buff, sizeof(buff)) == 4);
	TEST(strcmp(buff, "test") == 0);

	if (client) {
		close(client);
	}
	return 0;
}

int test_recv_reconnect(struct rsocket *rsocket)
{
	int client = 0;
	ACCEPT(client);
	if (client) {
		close(client);
	}

	// Make sure the other side knows it's closed
	char buff[5];
	while (1) {
		int err = rsocket_recv(rsocket, buff, sizeof(buff) - 1);
		if (err == -1) {
			rsocket_connect(rsocket);
			break;
		}
	}

	ACCEPT(client);

	send(client, "test", 4, MSG_NOSIGNAL);

	memset(buff, 0, sizeof(buff));
	TEST(rsocket_recv(rsocket, buff, sizeof(buff) - 1) == 4);
	TEST(strcmp(buff, "test") == 0);

	if (client) {
		close(client);
	}
	return 0;
}


void setup_server(struct rsocket *rsocket)
{
	int             ret = rsocket_open(HOST, PORT, rsocket);
	if (ret != 0) {
		exit(1);
	}
	server = rsocket->socket;

	int on = 1;
	if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	if (bind(server, rsocket->curr_addr->ai_addr, rsocket->curr_addr->ai_addrlen) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(server, 100) == -1) {
		perror("listen");
		exit(1);
	}
}

int test(test_fn fn)
{
	struct rsocket rsocket;
	TEST(rsocket_open(HOST, PORT, &rsocket) == 0);
	TEST(rsocket_connect(&rsocket) == 0);

	total_tests++;
	failed_tests += fn(&rsocket);

	rsocket_close(&rsocket);

	return 0;
}

int main(int argc, char **argv)
{
	struct rsocket rsocket;
	setup_server(&rsocket);

	printf("Running tests:\n\n");

	test(test_normal);
	test(test_send_reconnect);
	test(test_recv_reconnect);

	printf("\nResults: %u/%u passing (%u/%u conditions passing)\n",
		total_tests - failed_tests,
		total_tests,
		total - failed,
		total
		);

	rsocket_close(&rsocket);

	return failed != 0;
}

