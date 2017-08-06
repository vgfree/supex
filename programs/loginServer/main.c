#include <assert.h>
#include <signal.h>

#include "daemon.h"
#include "zmq_io_wraper.h"
#include "comm_io_wraper.h"
#include "upstream.h"
#include "libmini.h"


#define SERVER_FILE     "loginServer.pid"
#define MODULE_NAME     "loginServer"

int message_work(void)
{
	assert(init_comm_io() == 0);
	assert(init_zmq_io() == 0);

	while (1) {
		x_printf(D, "message_fountain.");
		upstream_msg();
	}

	exit_comm_io();
	exit_zmq_io();
	return 0;
}

int main(int argc, char *argv[])
{
	//signal(SIGPIPE, SIG_IGN);

	//if (daemon_init(SERVER_FILE) == 1) {
	//	printf("server is running");
	//	return -1;
	//}

	/*init log*/
	SLogOpen(MODULE_NAME ".log", SLogIntegerToLevel(1));
	
	message_work();

	daemon_exit(SERVER_FILE);
	return 0;
}

