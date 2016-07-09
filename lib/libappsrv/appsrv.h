#ifndef _APP_SRV_H_
#define _APP_SRV_H_

#include "zmq.h"

#include <sys/uio.h>

#define MAX_SPILL_DEPTH    32

typedef unsigned int ct_type;
struct app_msg
{
	size_t          vector_size;
	struct iovec    vector[MAX_SPILL_DEPTH];
};

void create_io(ct_type connect_type);

void destroy_io();

int app_recv_msg(struct app_msg *msg, int* more, int flag);

int app_send_msg(struct app_msg *msg);
	
#if 0
int app_send_msg(struct app_msg *msg);

int app_send_to_api(struct app_msg *msg);

int app_send_to_gateway(struct app_msg *msg);

int app_recv_all_msg(struct app_msg *msg, int *more, int flag);

int app_recv_login_msg(struct app_msg *msg, int flag);

int app_recv_gateway_msg(struct app_msg *msg, int flag);
#endif

#endif	/* ifndef _APP_SRV_H_ */

