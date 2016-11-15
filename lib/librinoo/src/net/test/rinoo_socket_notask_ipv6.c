/**
 * @file rinoo_socket_notask_ipv6.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2014
 * @date Sun Jan 3 15:34:47 2010
 *
 * @brief Test file for read/write functions without any task.
 *
 *
 */
#include "rinoo/rinoo.h"

extern const t_socket_class socket_class_tcp6;

void *server_thread(void *unused(arg))
{
	char b;
	t_sched *sched;
	t_socket *server;
	t_socket *client;
	struct sockaddr_in6 addr = { 0 };

	sched = rinoo_sched();
	XTEST(sched != NULL);
	server = rinoo_socket(sched, &socket_class_tcp6);
	XTEST(server != NULL);
	addr.sin6_port = htons(4242);
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = in6addr_any;
	XTEST(rinoo_socket_bind(server, (struct sockaddr *) &addr, sizeof(addr), 42) == 0);
	rinoo_log("server listening...");
	client = rinoo_socket_accept(server, NULL, NULL);
	XTEST(client != NULL);
	rinoo_log("server - sending 'abcdef'");
	XTEST(rinoo_socket_write(client, "abcdef", 6) == 6);
	rinoo_log("server - receiving 'b'");
	XTEST(rinoo_socket_read(client, &b, 1) == 1);
	XTEST(b == 'b');
	rinoo_log("server - receiving nothing");
	XTEST(rinoo_socket_read(client, &b, 1) == -1);
	rinoo_socket_destroy(client);
	rinoo_socket_destroy(server);
	rinoo_sched_destroy(sched);
	return NULL;
}

/**
 * Main function for this unit test.
 *
 * @return 0 if test passed
 */
int main()
{
	char a;
	char cur;
	pthread_t thread;
	t_sched *sched;
	t_socket *socket;
	struct sockaddr_in6 addr = { 0 };

	pthread_create(&thread, NULL, server_thread, NULL);
	sleep(1);
	sched = rinoo_sched();
	XTEST(sched != NULL);
	socket = rinoo_socket(sched, &socket_class_tcp6);
	XTEST(socket != NULL);
	addr.sin6_port = htons(4242);
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = in6addr_loopback;
	rinoo_log("client - connecting");
	XTEST(rinoo_socket_connect(socket, (struct sockaddr *) &addr, sizeof(addr)) == 0);
	rinoo_log("client - connected");
	for (cur = 'a'; cur <= 'f'; cur++) {
		rinoo_log("client - receiving '%c'", cur);
		XTEST(rinoo_socket_read(socket, &a, 1) == 1);
		XTEST(a == cur);
	}
	rinoo_log("client - sending 'b'");
	XTEST(rinoo_socket_write(socket, "b", 1) == 1);
	rinoo_socket_destroy(socket);
	rinoo_sched_destroy(sched);
	pthread_join(thread, NULL);
	XPASS();
}
