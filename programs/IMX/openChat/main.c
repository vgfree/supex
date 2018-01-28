#include "libmini.h"
#include "gateway/stream/stream_gateway.h"
#include "gateway/status/status_gateway.h"
#include "gateway/manage/manage_gateway.h"
#include "exchange/exchange.h"

int main(int argc, char *argv[])
{
	/*init log*/
	SLogOpen("openChat.log", SLogIntegerToLevel(0));

	status_gateway_work();
	stream_gateway_work();
	manage_gateway_work();
	exchange_work();

	exchange_wait();
	stream_gateway_wait();
	status_gateway_wait();
	manage_gateway_wait();

	exchange_stop();
	stream_gateway_stop();
	status_gateway_stop();
	manage_gateway_stop();
	return 0;
}

