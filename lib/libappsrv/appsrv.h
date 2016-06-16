#ifndef _APP_SRV_H_
#define _APP_SRV_H_

#include "zmq.h"

#include <sys/uio.h>

#define MAX_SPILL_DEPTH 32

struct app_msg
{
	size_t          vector_size;
	struct iovec    vector[MAX_SPILL_DEPTH];
};

void create_io();

void destroy_io();

int send_app_msg(struct app_msg *msg);

int recv_app_msg(struct app_msg *msg, int *more, int flag);
#endif /* ifndef _APP_SRV_H_ */

