#include "comm_message_operator.h"
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

static void _handle_cid_message(struct comm_message *msg)
{
  int fsz;
  char *cid = get_msg_frame(2, msg, &fsz);
  if (memcmp(cid, g_serv_info.ip, 4) == 0) {
    int fd = cid[4] * 256 + cid[5];
    set_msg_fd(msg, fd);
    remove_first_nframe(3, msg);
    log("fd:%d.", fd);
    comm_send(g_serv_info.commctx, msg, true, -1);
  }
}

static int _handle_gid_message(struct comm_message *msg)
{
  int fsz = 0;
  char *frame = get_msg_frame(2, msg, &fsz);
  char gid[20] = {};
  memcpy(gid, frame, fsz);
  int fd_list[GROUP_SIZE] = {};
  int size = find_fd_list(gid, fd_list);
  remove_first_nframe(3, msg);
  for (int i = 0; i < size; i++) {
    set_msg_fd(msg, fd_list[i]);
    comm_send(g_serv_info.commctx, msg, true, -1);
  }
  return 0;
}

static int _handle_uid_message(struct comm_message *msg)
{
  int fsz = 0;
  char *frame = get_msg_frame(2, msg, &fsz);
  char uid[20] = {};
  memcpy(uid, frame, fsz);
  int fd = find_fd(uid);
  if (fd != -1) {
    remove_first_nframe(3, msg);
    set_msg_fd(msg, fd);
    comm_send(g_serv_info.commctx, msg, true, -1);
  }
  return 0;
}

static int _handle_server_login(struct comm_message *msg)
{
  int fsz = 0;
  char *frame = get_msg_frame(2, msg, &fsz);
  if (memcmp(frame, "messageGateway", 14) == 0) {
    struct fd_node node;
    node.fd = msg->fd;
    list_push_back(MESSAGE_GATEWAY, &node);
  }
  else {
    char server[30] = {};
    memcpy(server, frame, fsz);
    error("frame size:%d, not support this server:%s", fsz, server);
  }
}

static int _handle_client_login(struct comm_message *msg)
{
  /* 即验证当前发送者.
     重新打包生成CID 并告知messageGateway. 
     客户端应在连接上coreChangeNode 时，发送第一个包为身份验证包。*/
  // 重新组包， 并生成cid , 通知server。
  int store_fd = msg->fd;
  // TO DO: 存储到redis.
  int fd;
  char cid[6] = {};
  memcpy(cid, g_serv_info.ip, 4);
  cid[4] = store_fd % 256;
  cid[5] = store_fd / 256;
  find_best_gateway(&fd);
  remove_first_nframe(2, msg);
  set_msg_fd(msg, fd);
  set_msg_frame(0, msg, 6, cid);
  set_msg_frame(0, msg, 3, "cid");
  set_msg_frame(0, msg, 5, "login");
  int ret = comm_send(g_serv_info.commctx, msg, true, -1);
  return ret;
}

static int handle_uid_map(struct router_head *head)
{
  assert(head);
  if (head->body_size != 4) {
    error("wrong body_size:%d", head->body_size);
    return -1;
  }
  if (memcmp(g_serv_info.ip, head->body, 4) != 0) {
    error("abandon message, this message is not belong to this core exchange node.");
    return -1;
  }
  char uid[UID_SIZE + 1];
  memcpy(uid, head->identity.Uid.uid, UID_SIZE);
  uid[UID_SIZE] = '\0';
  int fd = head->body[5];
  fd = fd * 256 + head->body[4];
  if (insert_fd(uid, fd) == -1) {
    error("insert error, uid:%s-------fd:%x.", uid, fd);
    return -1;
  }
  return 0;
}

static int handle_gid_map(struct router_head *head)
{
  assert(head);
  int cid = head->body_size / 6;
  if (head->body_size == 0 || head->body_size % 6 != 0 ) {
    error("wrong head->body_size:%x", head->body_size);
    return -1;
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
  memcpy(gid, head->identity.Gid.gid, GID_SIZE);
  gid[GID_SIZE] = '\0';
  if (insert_fd_list(gid, fdlist, count) == -1) {
    error("insert group fd, error.");
    return -1;
  }
  return 0;
}

static void _downstream_msg(struct comm_message *msg)
{
  int fsz;
  char *frame = get_msg_frame(1, msg, &fsz);
  if (!frame) {
    error("wrong frame, frame is NULL.");
    return;
  }
  if (fsz != 3) {
	error("downstream 2nd frame size:%d is not equal 3.", fsz);
  }
  if (memcmp(frame, "cid", 3) == 0) {
    _handle_cid_message(msg); 
  }
  else if (memcmp(frame, "uid", 3) == 0) {
    _handle_uid_message(msg);
  }
  else if (memcmp(frame, "gid", 3) == 0) {
    _handle_gid_message(msg);
  }
  else {
    error("wrong frame.");
  }
  return;
}

static void _classified_message(struct comm_message *msg)
{
  log("max msg:%d.", get_max_msg_frame(msg));
  int frame_size;
  char *frame = get_msg_frame(0, msg, &frame_size);
  if (!frame) {
    error("wrong frame, and frame is NULL.");
  }
  if (memcmp(frame, "downstream", 10) == 0) {
    _downstream_msg(msg);
  }
  else {
    error("wrong first frame, frame_size:%d.", frame_size);
  }
}

void message_dispatch()
{
  struct comm_message msg = {}; 
  init_msg(&msg);
  log("comm_recv wait.");
  comm_recv(g_serv_info.commctx, &msg, true, -1);
  struct fd_descriptor des;
  array_at_fd(msg.fd, &des);
  if (des.obj == CLIENT) {
    int fd;
    find_best_gateway(&fd);
    set_msg_fd(&msg, fd);
    comm_send(g_serv_info.commctx, &msg, true, -1);
  }
  else if (des.obj == MESSAGE_GATEWAY) { 
    _classified_message(&msg);
  }
  destroy_msg(&msg);
}
