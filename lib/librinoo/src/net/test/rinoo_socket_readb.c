/**
 * @file   rinoo_socket_readb.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu May  9 13:48:49 2013
 *
 * @brief  Test file for rinoo_socket_readb.
 *
 *
 */

#include "rinoo/rinoo.h"

#define TRANSFER_SIZE	(1024 * 1000)

static char *str;

void process_client(void *socket)
{
	t_buffer *buffer;

	buffer = buffer_create(NULL);
	XTEST(buffer != NULL);
	while (rinoo_socket_readb(socket, buffer) > 0) {
		rinoo_log("receiving...");
	}
	XTEST(buffer_size(buffer) == TRANSFER_SIZE);
	XTEST(buffer_strncmp(buffer, str, TRANSFER_SIZE) == 0);
	buffer_destroy(buffer);
	free(str);
	rinoo_socket_destroy(socket);
}

void server_func(void *unused(arg))
{
	t_socket *server;
	t_socket *client;

	server = rinoo_tcp_server(rinoo_sched_self(), IP_ANY, 4242);
	XTEST(server != NULL);
	client = rinoo_tcp_accept(server, NULL, NULL);
	XTEST(client != NULL);
	rinoo_log("client accepted");
	rinoo_task_start(rinoo_sched_self(), process_client, client);
	rinoo_socket_destroy(server);
}

void client_func(void *unused(arg))
{
	t_buffer buffer;
	t_socket *client;

	client = rinoo_tcp_client(rinoo_sched_self(), IP_LOOPBACK, 4242, 0);
	XTEST(client != NULL);
	str = malloc(sizeof(*str) * TRANSFER_SIZE);
	XTEST(str != NULL);
	memset(str, 'a', TRANSFER_SIZE);
	buffer_static(&buffer, str, TRANSFER_SIZE);
	XTEST(rinoo_socket_writeb(client, &buffer) == TRANSFER_SIZE);
	rinoo_socket_destroy(client);
}

/**
 * Main function for this unit test.
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_sched *sched;

	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_task_start(sched, server_func, NULL) == 0);
	XTEST(rinoo_task_start(sched, client_func, NULL) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	XPASS();
}
