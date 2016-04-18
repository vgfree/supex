#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <stdlib.h>
#include <stdint.h>

enum router_object {
  CLIENT = 0,
  MESSAGE_GATEWAY,
  ROUTER_SERVER
};

struct CID {
  char IP[4];
  int fd;
};

struct router_head {
  enum router_object message_from;
  enum router_object message_to;
  uint16_t CID_number;
  struct CID* cid;
  uint32_t body_size;
};

struct router_head *parse_router(char *data, uint32_t size);
char *pack_router(char *data, uint32_t *size, const struct router_head *head);

#endif
