#include <string.h>

#include "skt.h"

char data[] = "a1234567890";

void *work_task(void *args)
{
	struct skt_device       devc = {};
	int                     idx = 0;

	while (idx++ < 1000000) {
		zmq_cli_spill(&devc, data, strlen(data));
		zmq_cli_flush(&devc);
	}

	sleep(10);
}

int main(int argc, char *argv[])
{
	skt_register(argv [1]);
	zmq_cli_init("127.0.0.1", 5558);

#ifdef SELECT_MULTITHREAD
	zmq_threadstart(work_task, NULL);
	zmq_threadstart(work_task, NULL);
#else
	zmq_process_start(work_task, NULL);
	zmq_process_start(work_task, NULL);
#endif
	sleep(20);
	zmq_cli_exit();
}

