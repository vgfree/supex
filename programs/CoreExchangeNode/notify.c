#include "fd_manager.h"
#include "loger.h"
#include "message_dispatch.h"
#include "notify.h"

static void send_status_msg(int clientfd, int status)
{
  struct comm_message msg = {};
  init_msg(&msg);
  char cid[6] = {};
  memcpy(cid, g_serv_info.ip, 4);
  cid[4] = clientfd / 256;
  cid[5] = clientfd % 256;
  set_msg_frame(0, &msg, 6, cid);
  if (status == FD_INIT) {
    set_msg_frame(0, &msg, 9, "connected");
  }
  else if (status == FD_CLOSE) {
    set_msg_frame(0, &msg, 6, "closed");
  }
  set_msg_frame(0, &msg, 6, "status");
  int fd = 0;
  find_best_gateway(&fd);
  set_msg_fd(&msg, fd);
  comm_send(g_serv_info.commctx, &msg, true, -1);   
  destroy_msg(&msg);
}
void client_event_notify(struct comm_context *commctx,
                         struct portinfo *portinfo, void *usr)
{
  if (g_serv_info.commctx != commctx) {
    error("callback commctx not equal. g_serv_info.commctx:%p, commctx:%p",
          g_serv_info.commctx, commctx);
  }
  log("callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
  switch (portinfo->stat) {
  case 0:
    break;
  case FD_INIT:   // connected.
    {
      struct fd_descriptor des = {};
      des.status = 1;
      des.obj = CLIENT;
      array_fill_fd(portinfo->fd, &des);
      //send cid to messageGateway.
      send_status_msg(portinfo->fd, FD_INIT);
	  // refresh redis.
    }
    break;
  case FD_CLOSE:   // closed.
    {
      struct fd_descriptor des;
      array_at_fd(portinfo->fd, &des);
      log("array_at_fd, status:%d, obj:%d.", des.status, des.obj);
      if (des.status != 1) {
        error("this fd:%d is not running.", portinfo->fd);
        return;
      }
      send_status_msg(portinfo->fd, FD_CLOSE);
      // TO DO:  更新到redis 服务器。
      array_remove_fd(portinfo->fd);
      log("array_remove_fd.");
      break;
    }
  default:
    break;
  }
}

void server_event_notify(struct comm_context *commctx,
                         struct portinfo *portinfo, void *usr)
{
  if (g_serv_info.commctx != commctx) {
    error("callback commctx not equal. g_serv_info.commctx:%p, commctx:%p",
          g_serv_info.commctx, commctx);
  }
  log("callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
  switch (portinfo->stat) {
  case 0:
    break;
  case 1:   // connected.
    {
      struct fd_descriptor des = {};
      des.status = 1;
      des.obj = MESSAGE_GATEWAY;
      array_fill_fd(portinfo->fd, &des);
      struct fd_node node = {};
      node.fd = portinfo->fd;
      node.status = 1;
      list_push_back(MESSAGE_GATEWAY, &node);
    }
    break;
  case 4:   // closed.
    {
      struct fd_descriptor des;
      array_at_fd(portinfo->fd, &des);
      log("array_at_fd, status:%d, obj:%d.", des.status, des.obj);
      if (des.status != 1) {
        error("this fd:%d is not running.", portinfo->fd);
        return;
      }
      array_remove_fd(portinfo->fd);
      log("array_remove_fd.");
      list_remove(MESSAGE_GATEWAY, portinfo->fd);
      break;
    }
  default:
    break;
  }
}


