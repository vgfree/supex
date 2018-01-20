#pragma once

#include "comm_api.h"
#include "exc_comm_def.h"

struct server_info
{
	char                    host[20];	// 客户端绑定ip.
	uint16_t                port;	// 客户端绑定port
	struct comm_context     *commctx;

	int                     message_gateway_fd;	// message gateway 的fd.
	int                     control_gateway_fd;
	int                     login_gateway_fd;
};

extern struct server_info g_serv_info;

void get_cid(char cid[MAX_CID_SIZE], const int fd);

void exc_message_dispatch(void);

