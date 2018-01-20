#pragma once

#include "comm_api.h"

void make_uuid(char *dst);

void exc_event_notify_from_client(void *ctx, int socket, enum STEP_CODE step, void *usr);

void exc_event_notify_from_message(void *ctx, int socket, enum STEP_CODE step, void *usr);

void exc_event_notify_from_control(void *ctx, int socket, enum STEP_CODE step, void *usr);

void exc_event_notify_from_login(void *ctx, int socket, enum STEP_CODE step, void *usr);
