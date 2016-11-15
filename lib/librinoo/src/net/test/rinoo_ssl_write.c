/**
 * @file   rinoo_ssl_write.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Jun 21 14:52:08 2012
 *
 * @brief  Test file for rinoossl function
 *
 *
 */

#include	"rinoo/rinoo.h"

t_sched *sched;

void process_client(void *arg)
{
	char b;
	t_socket *socket = arg;

	rinoo_log("server - client accepted");
	rinoo_log("server - sending 'abcdef'");
	XTEST(rinoo_socket_write(socket, "abcdef", 6) == 6);
	rinoo_log("server - receiving 'b'");
	XTEST(rinoo_socket_read(socket, &b, 1) == 1);
	XTEST(b == 'b');
	rinoo_log("server - receiving nothing");
	XTEST(rinoo_socket_read(socket, &b, 1) == -1);
	rinoo_socket_destroy(socket);
}

void server_func(void *arg)
{
	t_ip fromip;
	uint32_t fromport;
	t_socket *client;
	t_socket *server;
	t_ssl_ctx *ctx = arg;

	server = rinoo_ssl_server(sched, ctx, 0, 4242);
	rinoo_log("server listening...");
	client = rinoo_ssl_accept(server, &fromip, &fromport);
	XTEST(client != NULL);
	rinoo_log("server - accepting client (%s:%d)", inet_ntoa(*(struct in_addr *) &fromip), fromport);
	rinoo_task_start(sched, process_client, client);
	rinoo_socket_destroy(server);
}

void client_func(void *arg)
{
	char a;
	char cur;
	t_socket *client;
	t_ssl_ctx *ctx = arg;

	rinoo_log("client - connecting...");
	client = rinoo_ssl_client(sched, ctx, 0, 4242, 0);
	XTEST(client != NULL);
	rinoo_log("client - connected");
	for (cur = 'a'; cur <= 'f'; cur++) {
		rinoo_log("client - receiving '%c'", cur);
		XTEST(rinoo_socket_read(client, &a, 1) == 1);
		XTEST(a == cur);
	}
	rinoo_log("client - sending 'b'");
	XTEST(rinoo_socket_write(client, "b", 1) == 1);
	rinoo_socket_destroy(client);
}


/**
 * Main function for this unit test.
 *
 * @return 0 if test passed
 */
int main()
{
	t_ssl_ctx *ssl;

	sched = rinoo_sched();
	XTEST(sched != NULL);
	ssl = rinoo_ssl_context();
	XTEST(ssl != NULL);
	rinoo_task_start(sched, server_func, ssl);
	rinoo_task_start(sched, client_func, ssl);
	rinoo_sched_loop(sched);
	rinoo_ssl_context_destroy(ssl);
	rinoo_sched_destroy(sched);
	XPASS();
}
