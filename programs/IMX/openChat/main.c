#include "libmini.h"
#include "gateway/message/message_gateway.h"
#include "gateway/login/login_gateway.h"
#include "gateway/control/control_gateway.h"
#include "exchange/exchange.h"

int main(int argc, char *argv[])
{
	/*init log*/
	SLogOpen("openChat.log", SLogIntegerToLevel(1));

	exchange_work();
	message_gateway_work();
	login_gateway_work();
	control_gateway_work();

	exchange_wait();
	message_gateway_wait();
	login_gateway_wait();
	control_gateway_wait();

	exchange_stop();
	message_gateway_stop();
	login_gateway_stop();
	control_gateway_stop();
	return 0;
}

