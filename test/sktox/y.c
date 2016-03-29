#include <string.h>

#include "skt.h"

int main(int argc, char *argv[])
{
	skt_register(argv [1]);
	zmq_trs_init();

	zmq_trs_add_inport(5558);
	zmq_trs_add_inport(5559);

	zmq_trs_add_export(5556);
	zmq_trs_add_export(5555);

	zmq_trs_start();
	sleep(100);
	zmq_trs_exit();
}

