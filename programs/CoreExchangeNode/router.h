#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <stdlib.h>
#include <stdint.h>

#define UID_SIZE 6
#define GID_SIZE 6

enum router_object {
  CLIENT = 0,
  MESSAGE_GATEWAY,
  ROUTER_SERVER,
  FULL_DAMS
};

struct CID {
  char IP[4];
  int fd;
};

struct GID {
  char gid[GID_SIZE];
};

struct UID {
  char uid[UID_SIZE];
};

struct router_head {
  enum router_object message_from;
  enum router_object message_to;
  uint8_t type;   /* 0x00 代表为用户Cid消息包, 0x01 为用户组Gid消息包, 0x02为用户Uid消息包，
		     0x03 为客户端连接验证包， 0x04 为业务服务器端连接验证包。0x05 uid----cid表, 0x06 gid ---- cid 表*/
  union {
    struct CID Cid;
    struct GID Gid;
    struct UID Uid;
  };
  uint32_t body_size;
  char *body;
};

struct router_head *parse_router(char *data, uint32_t size);
char *pack_router(const struct router_head *head);

#endif
