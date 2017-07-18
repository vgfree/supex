#ifndef _NOTIFY_H_
#define _NOTIFY_H_
#include "comm_api.h"

void make_uuid(char *dst);

void client_event_notify(struct comm_context *commctx,
	struct comm_tcp *portinfo, void *usr);

void message_gateway_event_notify(struct comm_context *commctx,
	struct comm_tcp *portinfo, void *usr);

void setting_server_event_notify(struct comm_context *commctx,
	struct comm_tcp *portinfo, void *usr);

void login_server_event_notify(struct comm_context *commctx,
	struct comm_tcp *portinfo, void *usr);

#endif

