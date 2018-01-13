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

	message_gateway_work();
	message_gateway_wait();
	message_gateway_stop();
	return 0;
}

