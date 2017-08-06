#include "daemon.h"
#include "libmini.h"
#include "comm_message_operator.h"
#include "comm_io_wraper.h"
#include "downstream.h"
#include "zmq_io_wraper.h"

#include <pthread.h>
#include <signal.h>

#define SERVER_FILE     "settingServer.pid"
#define MODULE_NAME     "settingServer"

void message_work(void)
{
	assert(init_comm_io() == 0);
	assert(init_zmq_io() == 0);
	while (1) {
		struct comm_message msg = {};
		init_msg(&msg);
		pull_msg(&msg);
		downstream_msg(&msg);
		destroy_msg(&msg);
	}
	exit_comm_io();
	exit_zmq_io();
}



int main(int argc, char *argv[])
{
	//signal(SIGPIPE, SIG_IGN);

	//if (daemon_init(SERVER_FILE) == 1) {
	//	printf("server is running");
	//	return -1;
	//}

	message_work();
	
	daemon_exit(SERVER_FILE);
	return 0;
}

