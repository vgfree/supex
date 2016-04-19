#include "fd_manager.h"
#include "message_dispatch.h"
#include "router.h"
#include "loger.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct server_info g_serv_info = {};

// 专门用于来自于客户端的数据。
static void compose_new_packet(const struct router_head *head,
                               const struct comm_message *oldmsg,
                               struct comm_message *newmsg)
{
  struct CID *cid = (struct CID*)malloc(sizeof(struct CID));
  struct router_head rhead;
  memcpy(cid->IP, g_serv_info.ip, 4);
  cid->fd = oldmsg->fd;
  rhead.message_from = head->message_from;
  rhead.message_to = head->message_to;
  rhead.CID_number = 1;
  rhead.cid = cid;
  rhead.body_size = head->body_size;
  char *pos = oldmsg->content + (oldmsg->size - head->body_size);
  uint32_t size;
  newmsg->content = pack_router(pos, &size, &rhead);
  newmsg->size = size;
  printf("size:%d.", size);
  free(cid);
}

void find_best_gateway(int *fd)
{
  struct fd_node node;
  if (list_front(MESSAGE_GATEWAY, &node) == FAILED) {
    error("no gateway server.");
  }
  *fd = node.fd;
}

static void handle_client_message(const struct router_head *head,
                                  const struct comm_message *msg)
{
  // 重新往mfptp 中添加包头，路由到指定服务器。  
  switch (head->message_to) {
  case CLIENT:
    log("Not support client to client.");
    break;
  case MESSAGE_GATEWAY:
    {
      struct comm_message new_msg;
      compose_new_packet(head, msg, &new_msg);
      int fd;
      find_best_gateway(&fd);
      new_msg.fd = fd;
      comm_send(g_serv_info.commctx, &new_msg);
      free(new_msg.content);
	}
    break;
  case ROUTER_SERVER:
	// 定义所有目的地为路由服务器的包为身份验证包， 即验证当前发送者.
	// 重新打包生成CID 并告知messageGateway. 
	// 客户端应在连接上coreChangeNode 时，发送第一个包为身份验证包。
	{
      struct comm_message new_msg;
      compose_new_packet(head, msg, &new_msg);
	  int fd;
	  find_best_gateway(&fd);
	  new_msg.fd = fd;
	  comm_send(g_serv_info.commctx, &new_msg);
	  free(new_msg.content);
	}
    break;
  default:
    break;
  }
}

static void handle_gateway_message(const struct router_head *head,
                                   const struct comm_message *msg)
{
  switch (head->message_to) {
  case CLIENT:
    {
      for (int i = 0; i < head->CID_number; i++) {
        struct comm_message new_msg;
        new_msg.fd = head->cid[i].fd;
        new_msg.size = head->body_size;
        new_msg.content = (char *)malloc(head->body_size * sizeof(char));
        log("message body start :%d", msg->size - head->body_size);
        memcpy(new_msg.content, msg->content + (msg->size - head->body_size), head->body_size);
        log("Prepare to send mesg.");
        comm_send(g_serv_info.commctx, &new_msg);
        free(new_msg.content);
	  }
	}
    break;
  case MESSAGE_GATEWAY:
    log("Not support message_gateway to message_gateway.");
    break;
  case ROUTER_SERVER:
    // 每个messageGateway 连接上该路由server时，发送的第一个包应为身份验证包，
	// 也就只包含唯一的路由帧。
	{
      struct fd_node node;
      node.fd = msg->fd;
      list_push_back(MESSAGE_GATEWAY, &node);
	}
    break;
  default:
    break;
  }
}

void message_dispatch()
{
  struct comm_message org_msg; 
  org_msg.content = (char *)malloc(g_serv_info.package_size * sizeof(char));
  comm_recv(g_serv_info.commctx, &org_msg);
  if (org_msg.fd != 0) {
    log("org_msg fd:%d, size:%d", org_msg.fd, org_msg.size);
    for (int i = 0; i < org_msg.size; i++) {
      log("%x,", org_msg.content[i]);
    }
    struct router_head *oldhead = 
      parse_router(org_msg.content, org_msg.size);

    log("head, message_from:%d, message_to:%d.", oldhead->message_from, oldhead->message_to);
    switch (oldhead->message_from) {
    case CLIENT:
      handle_client_message(oldhead, &org_msg);
      break;
    case MESSAGE_GATEWAY:
      handle_gateway_message(oldhead, &org_msg);
      break;
    default:
      error("msg head data (message_from :%d) is not right.",
            oldhead->message_from);
    }
    free(oldhead);
  }
  else {
    log("no message.");
  }
  free(org_msg.content);
}
