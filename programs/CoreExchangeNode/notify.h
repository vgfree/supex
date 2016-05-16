#ifndef _NOTIFY_H_
#define _NOTIFY_H_
#include "communication.h"

void client_event_notify(struct comm_context *commctx,
                         struct portinfo *portinfo, void *usr);
void message_gateway_event_notify(struct comm_context *commctx,
  struct portinfo *portinfo, void *usr);
void setting_server_event_notify(struct comm_context *commctx,
  struct portinfo *portinfo, void *usr);
void login_server_event_notify(struct comm_context *commctx,
  struct portinfo *portinfo, void *usr);
#endif
