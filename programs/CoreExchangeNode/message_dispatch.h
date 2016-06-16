#ifndef _MESSAGE_DISPATCH_H_
#define _MESSAGE_DISPATCH_H_

#include "communication.h"

struct server_info
{
	char                    ip[20];	// 客户端绑定ip.
	uint16_t                port;	// 客户端绑定port
	struct comm_context     *commctx;
	uint32_t                package_size;
	int                     message_gateway_fd;	// message gateway 的fd.
	int                     setting_server_fd;
	int                     login_server_fd;
};

extern struct server_info g_serv_info;

void find_best_gateway(int *fd);

void message_dispatch();
#endif	/* ifndef _MESSAGE_DISPATCH_H_ */

