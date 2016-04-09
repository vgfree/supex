
#include "message_dispatch.h"
#include "router.h"

#include <stdint.h>
#include <stdlib.h>

extern uint32_t get_local_ip();
void cid_compose(uint32_t ip, int fd, struct CID *cid)
{
  cid = (struct CID*) malloc(sizeof(struct CID));
  cid->IP = ip;
  cid->fd = fd;
}

static void client_msg_pack(struct message *ready_msg, struct message *org_msg)
{
  ready_msg->message_head.message_from = ROUTER_SERVER;
  ready_msg->message_head.message_to = org_msg->message_head.message_to;  
  ready_msg->message_head.CID_number = 1;
  ready_msg->message_head.cid = (struct CID**) malloc(sizeof(struct CID*));
  cid_compose(get_local_ip(), org_msg->message_fd,
              ready_msg->message_head.cid[0]);
}

static void gateway_msg_pack(struct message *ready_msg, struct message *org_msg)
{
	
}

static void client_route_to_gateway(struct message *msg)
{
  // 从客户端来的消息需要路由去多台gateway, 还是寻找最好的gateway路由。
  int fd = find_router_object(MESSAGE_GATEWAY);
  msg->message_fd = fd;
  push_message(msg);
}

static void gateway_route_to_client(struct message *msg)
{
	
}

static void router_select(struct message *msg, enum router_object obj)
{
  switch (obj) {
  case CLIENT:
    client_route_to_gateway(msg);
    break;
  case MESSAGE_GATEWAY:
    gateway_route_to_client(msg);
    break;
  case ROUTER_SERVER:
    break;
  default:
    error("msg data() error, can't find message to server:%d .", obj);
  }
}

void message_dispatch()
{
  struct message* org_msg = (struct message*)malloc(sizeof(struct message));
  struct message* ready_msg = (struct message*)malloc(sizeof(struct message));
  pop_message(org_msg);
  switch (org_msg->message_head.message_from) {
  case CLIENT:
    client_msg_pack(ready_msg, org_msg);
    break;
  case MESSAGE_GATEWAY:
    gateway_msg_pack(ready_msg, org_msg);
    break;
  default:
    error("msg head data (message_from :%d) is not right.",
          org_msg->message_head.message_from);
  }
  router_select(ready_msg, ready_msg->message_head.message_to);
  free(org_msg);
  free(ready_msg);
}
