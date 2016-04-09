
#include "message_dispatch.h"
#include "router.h"

#include <stdint.h>
#include <stdlib.h>

extern uint32_t get_local_ip();
static void compose_cid(uint32_t ip, int fd, struct CID &cid)
{
  cid->IP = ip;
  cid->fd = fd;
}

static void get_router_head(const struct message &msg,
                            struct router_head &head)
{
  // to do:从mfptp 中获取第一帧的内容为路由头。 
}

static void compose_new_packet(const struct message &oldmsg,
							   struct message &newmsg)
{
  struct CID cid;
  struct router_head rhead;
  compose_cid(get_local_ip(), msg.message_fd, cid);
  //to do: 根据原来的oldmsg 重新组装新包。
}

static void find_best_gateway(int &fd)
{
  struct fd_node node;
  if (list_front(MESSAGE_GATEWAY, node) == FAILED) {
    error("no gateway server.");
  }
  fd = node.fd;
}

static void handle_client_message(const struct router_head &head,
								  const struct message &msg)
{
  // to do: 重新往mfptp 中添加包头，路由到指定服务器。  
  switch (head.message_to) {
  case CLIENT:
    break;
  case MESSAGE_GATEWAY:
	{
	  struct message new_msg;
	  compose_new_packet(msg, new_msg);
	  int fd;
      find_best_gateway(fd);
	  new_msg.message_fd = fd;
	  push_message(new_msg);
	}
    break;
  case ROUTER_SERVER:
	break;
  default:
  }
}

static handle_gateway_message(const struct router_head head,
							  const struct message &msg)
{
  switch (head.message_to) {
  case CLIENT:
	{
      for (int i = 0; i < head.CID_number; i++) {
        struct message new_msg;
		compose_new_packet(msg, new_msg);
		new_msg.message_fd = head.cid[i].fd;
		push_message(new_msg);
	  }
	}
    break;
  case MESSAGE_GATEWAY:
    break;
  case ROUTER_SERVER:
	break;
  default:
  }
}

void message_dispatch()
{
  struct message org_msg; 
  pop_message(org_msg);
  struct router_head head;
  get_router_head(org_msg, head);

  switch (head.message_from) {
  case CLIENT:
    handle_client_message(head, org_msg);
    break;
  case MESSAGE_GATEWAY:
    handle_gateway_message();
    break;
  default:
    error("msg head data (message_from :%d) is not right.",
          org_msg->message_head.message_from);
  }
}
