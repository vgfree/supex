#ifndef _CONNECT_OPER_H_
#define _CONNECT_OPER_H_
/**
 * author:lanjian
 * data:2016/07/08
 */

#include "appsrv.h"
void init_connect(void *ctx, int types);

void destroy_connect(void); 

int recv_msg(enum askt_type type, struct app_msg *msg);

int recv_more_msg(int types, struct app_msg *msg, int flag);

int send_msg(enum askt_type type, struct app_msg *msg);
#endif /* #define _CONNECT_OPER_H_*/
