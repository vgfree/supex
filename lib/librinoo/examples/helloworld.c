#include "rinoo/rinoo.h"

void task_client(void *socket)
{
	char a;

	rinoo_socket_write(socket, "Hello world!\n", 13);
	rinoo_socket_read(socket, &a, 1);
	rinoo_socket_destroy(socket);
}

void task_server(void *sched)
{
	t_socket *server;
	t_socket *client;

	server = rinoo_tcp_server(sched, IP_ANY, 4242);
	while ((client = rinoo_tcp_accept(server, NULL, NULL)) != NULL) {
		rinoo_task_start(sched, task_client, client);
	}
	rinoo_socket_destroy(server);
}

int main()
{
	t_sched *sched;

	sched = rinoo_sched();
	rinoo_task_start(sched, task_server, sched);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	return 0;
}
