#include "libmini.h"
#include "message_concentrator.h"

#include <pthread.h>
#include <signal.h>

#define SERVER_FILE     "messageGateway.pid"
#define MODULE_NAME     "messageGateway"

int main(void)
{
	/*init log*/
	SLogOpen(MODULE_NAME ".log", SLOG_D_LEVEL);
	x_printf(E, "--------------\n");

	concentrator_work();

	x_printf(W, "exit main thread.");
	concentrator_destroy();
	return 0;
}

