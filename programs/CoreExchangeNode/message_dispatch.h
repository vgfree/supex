#ifndef _MESSAGE_DISPATCH_H_
#define _MESSAGE_DISPATCH_H_

#include "communication.h"

struct server_info{
  char ip[4];
  uint16_t port;
  struct comm_context *commctx;
  uint32_t package_size;
  int fd;
};

extern struct server_info g_serv_info;

void find_best_gateway(int *fd);
void message_dispatch();

#endif
