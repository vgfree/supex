#include "rinoo/rinoo.h"

void task_client(void *socket)
{
	char a;

	rinoo_socket_write(socket, "Hello world!\n", 13);
	rinoo_socket_read(socket, &a, 1);
	rinoo_socket_destroy(socket);
}

void task_server(void *server)
{
	t_sched *sched;
	t_socket *client;

	sched = rinoo_sched_self();
	while ((client = rinoo_tcp_accept(server, NULL, NULL)) != NULL) {
		rinoo_log("Accepted connection on thread %d", sched->id);
		rinoo_task_start(sched, task_client, client);
	}
	rinoo_socket_destroy(server);
}

int main()
{
	int i;
	t_sched *spawn;
	t_sched *sched;
	t_socket *server;

	sched = rinoo_sched();
	/* Spawning 10 schedulers, each running in a separate thread */
	rinoo_spawn(sched, 10);
	for (i = 0; i <= 10; i++) {
		spawn = rinoo_spawn_get(sched, i);
		server = rinoo_tcp_server(spawn, IP_ANY, 4242);
		rinoo_task_start(spawn, task_server, server);
	}
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	return 0;
}
