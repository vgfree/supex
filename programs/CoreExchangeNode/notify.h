#ifndef _NOTIFY_H_
#define _NOTIFY_H_
#include "communication.h"

void client_event_notify(struct comm_context *commctx,
                         struct portinfo *portinfo, void *usr);
void server_event_notify(struct comm_context *commctx,
                         struct portinfo *portinfo, void *usr);
#endif
