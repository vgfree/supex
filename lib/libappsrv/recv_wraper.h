#ifndef _RECV_WRAPER_H_
#define _RECV_WRAPER_H_

#include "appsrv.h"

void init_recv(void *ctx);

void destroy_recv();

int recv_all_msg(struct app_msg *msg, int *more, int flag);

int recv_login_msg(struct app_msg *msg, int flag);

int recv_gateway_msg(struct app_msg *msg, int flag);
#endif

