#ifndef _SEND_WRAPER_H_
#define _SEND_WRAPER_H_

#include "appsrv.h"

#if 0
void init_send(void *ctx);

void destroy_send();
#endif

int send_to_api(struct app_msg *msg, void *ct);

int send_to_gateway(struct app_msg *msg, void *ct);
#endif

