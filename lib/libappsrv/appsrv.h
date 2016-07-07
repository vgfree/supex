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

int app_send_msg(struct app_msg *msg);

int app_send_to_api(struct app_msg *msg);

int app_send_to_gateway(struct app_msg *msg);

int app_recv_all_msg(struct app_msg *msg, int *more, int flag);

int app_recv_login_msg(struct app_msg *msg, int flag);

int app_recv_gateway_msg(struct app_msg *msg, int flag);
#endif	/* ifndef _APP_SRV_H_ */

