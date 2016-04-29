#ifndef _COMM_IO_WRAPER_H_
#define _COMM_IO_WRAPER_H_

#include "communication.h"

int init_comm_io();

int recv_msg(struct comm_message *msg);
int send_msg(struct comm_message *msg);
#endif
