#include "fd_manager.h"
#include "loger.h"
#include "message_dispatch.h"
#include "notify.h"

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
  case 1:   // connected.
    {
      struct fd_descriptor des = {};
      des.status = 1;
      des.obj = CLIENT;
      array_fill_fd(portinfo->fd, &des);
      // to do: send cid to messageGateway.
	  // refresh redis.
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
      // 打包路由结束包发送给其他服务器。message_from 为router_server.
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
      break;
    }
  default:
    break;
  }
}


