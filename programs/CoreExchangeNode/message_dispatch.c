#include "fd_manager.h"
#include "gid_map.h"
#include "loger.h"
#include "message_dispatch.h"
#include "router.h"
#include "uid_map.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct server_info g_serv_info = {};

void find_best_gateway(int *fd)
{
  struct fd_node node;
  if (list_front(MESSAGE_GATEWAY, &node) == FAILED) {
    error("no gateway server.");
  }
  *fd = node.fd;
}

static void handle_cid_message(struct router_head *head,
                               struct comm_message *msg)
{
// 去掉第一帧重新打包。
  struct comm_message new_msg;
  new_msg.fd = head->Cid.fd;
  new_msg.dsize = msg->dsize - msg->frame_offset[0];
  new_msg.frames = msg->frames - 1;
  for (int i = 0; i < new_msg.frames; i++) {
    new_msg.frame_offset[i] = msg->frame_offset[i+1];
  }
  new_msg.content = (char *)malloc(new_msg.dsize * sizeof(char));
  log("message body start :%d", msg->dsize - head->body_size);
  memcpy(new_msg.content, msg->content + msg->frame_offset[1], new_msg.dsize);
  log("Prepare to send mesg.");
  comm_send(g_serv_info.commctx, &new_msg, true, -1);
  free(new_msg.content);
}

static void handle_gid_message(struct router_head *head,
                               struct comm_message *msg)
{
  // To do:
}

static void handle_uid_message(struct router_head *head,
                               struct comm_message *msg)
{
  // To do:
}

static void handle_server_login(struct router_head *head,
                                struct comm_message *msg)
{
  switch (head->message_from) {
    case MESSAGE_GATEWAY: {
      struct fd_node node;
      node.fd = msg->fd;
      list_push_back(MESSAGE_GATEWAY, &node);
    }
    break;
	case ROUTER_SERVER:
      log("Not support router_server to router_server.");
    break;
    case FULL_DAMS:
      // to do. map table.
    break;
  } 
}

static void handle_client_login(struct router_head *head,
                                struct comm_message *msg)
{
  /* 即验证当前发送者.
     重新打包生成CID 并告知messageGateway. 
     客户端应在连接上coreChangeNode 时，发送第一个包为身份验证包。*/
  struct comm_message new_msg;
  // 重新组包， 并生成cid , 通知server。
  // TO DO: 存储到redis.
  int fd;
  find_best_gateway(&fd);
  new_msg.fd = fd;
  comm_send(g_serv_info.commctx, &new_msg, true, -1);
  free(new_msg.content);
}

static void handle_uid_map(struct router_head *head)
{
  assert(head);
  if (head->body_size != 4) {
    error("wrong body_size:%d", head->body_size);
    return;
  }
  if (memcmp(g_serv_info.ip, head->body, 4) != 0) {
    error("abandon message, this message is not belong to this core exchange node.");
    return;
  }
  char uid[UID_SIZE + 1];
  memcpy(uid, head->Uid.uid, UID_SIZE);
  uid[UID_SIZE] = '\0';
  int fd = head->body[5];
  fd = fd * 256 + head->body[4];
  if (insert_fd(uid, fd) == -1) {
    error("insert error, uid:%s-------fd:%x.", uid, fd);
  }
}

static void handle_gid_map(struct router_head *head)
{
  assert(head);
  int cid = head->body_size / 6;
  if (head->body_size == 0 || head->body_size % 6 != 0 ) {
    error("wrong head->body_size:%x", head->body_size);
  }
  int fdlist[GROUP_SIZE];
  int count = 0;
  for (int i = 0; i < cid; i++) {
    char uid[4];
    memcpy(uid, head->body + i * 6, 4);
    if (memcmp(g_serv_info.ip, uid, 4) != 0) {
      continue;
    }
    int fd = *(head->body + i * 6 + 5);
    fd = fd * 256 + *(head->body + i*6 + 4);
    fdlist[count++] = fd;
  }
  char gid[GID_SIZE + 1];
  memcpy(gid, head->Gid.gid, GID_SIZE);
  gid[GID_SIZE] = '\0';
  if (insert_fd_list(gid, fdlist, count) == -1) {
    error("insert group fd, error.");
  }
}

void message_dispatch()
{
  struct comm_message org_msg; 
  org_msg.content = (char *)malloc(g_serv_info.package_size * sizeof(char));
  comm_recv(g_serv_info.commctx, &org_msg, true, -1);
  if (org_msg.fd != 0) {
    log("org_msg fd:%d, size:%d", org_msg.fd, org_msg.dsize);
    struct router_head *oldhead = 
      parse_router(org_msg.content, org_msg.frame_offset[1]);
    if (oldhead) {
      switch (oldhead->type) {
        case 0x0:
          handle_cid_message(oldhead, &org_msg);
          break;
        case 0x01:
          handle_gid_message(oldhead, &org_msg);
          break;
        case 0x02:
          handle_uid_message(oldhead, &org_msg);
          break;
        case 0x03:
          handle_client_login(oldhead, &org_msg);
          break;
        case 0x04:
          handle_server_login(oldhead, &org_msg);
        case 0x05:
          handle_uid_map(oldhead);
          break;
        case 0x06:
          handle_gid_map(oldhead);
          break;
        default:
          break;
      }
      log("head type:%d, message_from:%d, message_to:%d.",
          oldhead->type, oldhead->message_from, oldhead->message_to);
      free(oldhead);
    }
    else {
      error("wrong data.");
    }
  }
  else {
    log("no message.");
  }
  free(org_msg.content);
}
