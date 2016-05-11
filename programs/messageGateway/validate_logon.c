#include "communication.h"
#include "comm_io_wraper.h"
#include "comm_message_operator.h"
#include "loger.h"
#include "validate_logon.h"

int send_validate_logon_package(int fd)
{
  struct comm_message msg = {};
  init_msg(&msg);
  set_msg_frame(0, &msg, 14, "messageGateway");
  set_msg_frame(0, &msg, 6, "server");
  set_msg_frame(0, &msg, 5, "login");
  set_msg_fd(&msg, fd);
  log("send msg, fd:%d.", fd);
  if (send_msg(&msg) == -1) {
    error("wron msg, msg fd:%d.", msg.fd);
  }
  destroy_msg(&msg);
  return 0;
}
