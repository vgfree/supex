#ifndef _CONNECT_OPER_H_
#define _CONNECT_OPER_H_
/**
 * author:lanjian
 * data:2016/07/08
 */

#include "appsrv.h"
void init_connect(void *ctx, ct_type connect_type);

void destroy_connect(); 

int recv_msg(struct app_msg *msg, int* more, int flag);

int send_msg(struct app_msg *msg); 
#endif /* #define _CONNECT_OPER_H_*/
