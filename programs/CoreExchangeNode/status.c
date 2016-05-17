#include "comm_message_operator.h"
#include "gid_map.h"
#include "message_dispatch.h"
#include "status.h"
#include "uid_map.h"

int erase_client(int fd)
{
  char uid[30] = {};
  int size = 0;
  find_uid(uid, &size, fd);
  remove_fd(uid);
  remove_uid(fd);
  char gid_list[20][20] = {};
  find_gid_list(fd, (char**)gid_list, &size);
  for (int i = 0; i < size; i++) {
    remove_fd_list((char*)gid_list[i], &fd, 1);
  }
  remove_gid_list(fd, (char**)gid_list, size);
  return size;
}

void send_status_msg(int clientfd, int status)
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
  set_msg_fd(&msg, g_serv_info.login_server_fd);
  comm_send(g_serv_info.commctx, &msg, true, -1);   
  destroy_msg(&msg);
}

