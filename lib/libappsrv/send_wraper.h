#ifndef _SEND_WRAPER_H_
#define _SEND_WRAPER_H_

#include "appsrv.h"

void init_send(void *ctx);

void destroy_send();

int send_to_api(struct app_msg *msg);

int send_to_gateway(struct app_msg *msg);
#endif

