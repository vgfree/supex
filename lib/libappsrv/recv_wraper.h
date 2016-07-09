#ifndef _RECV_WRAPER_H_
#define _RECV_WRAPER_H_

#include "appsrv.h"

#if 0
void init_recv(void *ctx);

void destroy_recv();
#endif

int recv_more_msg(struct app_msg *msg, void *ct_upstream, void *ct_recv_login, 
		int *more, int flag);

int recv_login_msg(struct app_msg *msg, void *ct_recv_login, int flag); 

int recv_gateway_msg(struct app_msg *msg, void *ct_upstream, int flag);
#endif

