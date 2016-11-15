/**
 * @file   http_server.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Sep 22 17:22:40 2013
 *
 * @brief  Example of a multi-threaded HTTP server.
 *	   $ gcc ./examples/http_server.c -o rinoo_httpserv -I./include -L. -lrinoo_static -lpthread
 *
 */

#include "rinoo/rinoo.h"

t_sched *master;

/**
 * Process HTTP requests for each client.
 *
 * @param arg Client socket
 */
void process_http_client(void *arg)
{
	t_http http;
	t_socket *client = arg;

	rinoo_http_init(client, &http);
	while (rinoo_http_request_get(&http)) {
		rinoo_log("Request uri=%.*s thread=%d", buffer_size(&http.request.uri), buffer_ptr(&http.request.uri), rinoo_sched_self()->id);
		http.response.code = 200;
		if (rinoo_http_response_send(&http, NULL) != 0) {
			goto error;
		}
		rinoo_http_reset(&http);
	}
	rinoo_http_destroy(&http);
	rinoo_socket_destroy(client);
	return;
error:
	rinoo_log("error");
	rinoo_http_destroy(&http);
	rinoo_socket_destroy(client);
}

/**
 * Accepts incoming connection on the HTTP server
 *
 * @param arg Server socket
 */
void process_http_server(void *arg)
{
	t_socket *client;
	t_socket *socket = arg;

	rinoo_log("Thread %d started.", rinoo_sched_self()->id);
	while ((client = rinoo_tcp_accept(socket, NULL, NULL)) != NULL) {
		rinoo_task_start(rinoo_sched_self(), process_http_client, client);
	}
	rinoo_socket_destroy(socket);
	rinoo_log("Thread %d stopped.", rinoo_sched_self()->id);
}

void signal_handler(int unused(signal))
{
	rinoo_log("Exiting...");
	rinoo_sched_stop(master);
}

/**
 * Main function
 *
 * @param argc Number of parameters
 * @param argv Parameters
 *
 * @return Should return 0
 */
int main(int argc, char **argv)
{
	int i;
	t_sched *cur;
	t_socket *server;

	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		return -1;
	}
	master = rinoo_sched();
	if (master == NULL) {
		return -1;
	}
	server = rinoo_tcp_server(master, IP_ANY, atoi(argv[1]));
	if (server == NULL) {
		rinoo_log("Could not create server socket on port %s.", argv[1]);
		rinoo_sched_destroy(master);
		return -1;
	}
	if (sigaction(SIGINT, &(struct sigaction){ .sa_handler = signal_handler }, NULL) != 0) {
		rinoo_sched_destroy(master);
		return -1;
	}
	/* Spawning 10 threads */
	if (rinoo_spawn(master, 10) != 0) {
		rinoo_log("Could not spawn threads.");
		rinoo_sched_destroy(master);
		return -1;
	}
	for (i = 1; i <= 10; i++) {
		cur = rinoo_spawn_get(master, i);
		rinoo_task_start(cur, process_http_server, rinoo_socket_dup(cur, server));
	}
	rinoo_task_start(master, process_http_server, server);
	rinoo_sched_loop(master);
	rinoo_sched_destroy(master);
	return 0;
}
