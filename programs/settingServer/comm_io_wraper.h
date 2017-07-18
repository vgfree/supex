#ifndef _COMM_IO_WRAPER_H_
#define _COMM_IO_WRAPER_H_

#include "comm_api.h"

#define NODE_SIZE 10

struct core_exchange_node
{
	int     fd_array[NODE_SIZE];
	int     max_size;
};

struct core_exchange_node *g_node_ptr;

int init_comm_io(void);

void exit_comm_io(void);

int recv_msg(struct comm_message *msg);

int send_msg(struct comm_message *msg);
#endif	/* ifndef _COMM_IO_WRAPER_H_ */

