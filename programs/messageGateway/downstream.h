#ifndef _DOWNSTREAM_H_
#define _DOWNSTREAM_H_
#include "communication.h"

int downstream_msg(struct comm_message *msg);

int pull_msg(struct comm_message *msg);
#endif

