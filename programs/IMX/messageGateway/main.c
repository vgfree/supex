#include "daemon.h"
#include "libmini.h"
#include "message_concentrator.h"

#include <pthread.h>
#include <signal.h>

#define SERVER_FILE     "messageGateway.pid"
#define MODULE_NAME     "messageGateway"

int main(void)
{
	//signal(SIGPIPE, SIG_IGN);

	//if (daemon_init(SERVER_FILE) == 1) {
	//	printf("server is running");
	//	return -1;
	//}

	/*init log*/
	SLogOpen(MODULE_NAME ".log", SLogIntegerToLevel(1));
	
	concentrator_work();

	x_printf(W, "exit main thread.");
	concentrator_destroy();
	daemon_exit(SERVER_FILE);
	return 0;
}

