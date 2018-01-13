#include "libmini.h"
#include "message_gateway.h"
#include "login_gateway.h"


int main(int argc, char *argv[])
{
	/*init log*/
	SLogOpen("gateway.log", SLogIntegerToLevel(1));

	message_gateway_work();
	login_gateway_work();

	message_gateway_wait();
	login_gateway_wait();

	message_gateway_stop();
	login_gateway_stop();
	return 0;
}

