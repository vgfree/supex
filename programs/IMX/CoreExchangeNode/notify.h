#ifndef _NOTIFY_H_
#define _NOTIFY_H_
#include "comm_api.h"

void make_uuid(char *dst);

void client_event_notify(void *ctx, int socket, enum STEP_CODE step, void *usr);

void message_gateway_event_notify(void *ctx, int socket, enum STEP_CODE step, void *usr);

void setting_server_event_notify(void *ctx, int socket, enum STEP_CODE step, void *usr);

void login_server_event_notify(void *ctx, int socket, enum STEP_CODE step, void *usr);

#endif

