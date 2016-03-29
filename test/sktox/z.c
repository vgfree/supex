#include <string.h>
#include <stdlib.h>

#include "skt.h"

char data[] = "a1234567890";

void *work_task(void *args)
{
	struct skt_device       devc = {};
	int                     idx = 0;

	while (1) {
		zmq_srv_fetch(&devc);

		int i = 0;

		for (i = 0; i < devc.idx; i++) {
			free(devc.ibuffer[i].iov_base);
		}

		idx++;
		printf("%d %d\n", args, idx);

		if ((idx % 1000000) == 0) {
			printf("100万\n");
		}
	}
}

int main(int argc, char *argv[])
{
	skt_register(argv [1]);
	zmq_srv_init("127.0.0.1", 5556);

#ifdef SELECT_MULTITHREAD
	zmq_threadstart(work_task, 1);
	zmq_threadstart(work_task, 2);
#else
	zmq_process_start(work_task, (void *)1);
	zmq_process_start(work_task, (void *)2);
#endif

	zmq_srv_start();

	sleep(100);
	zmq_srv_exit();
}

