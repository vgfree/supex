#include "communication.h"
#include "comm_io_wraper.h"
#include "comm_message_operator.h"
#include "loger.h"
#include "validate_logon.h"

int send_validate_logon_package(int fd)
{
  struct comm_message msg = {};
  msg.content = (char *)malloc(102400 * sizeof(char));
  set_msg_frame(0, &msg, 14, "messageGateway");
  set_msg_frame(0, &msg, 6, "server");
  set_msg_frame(0, &msg, 5, "login");
  set_msg_fd(&msg, fd);
  if (send_msg(&msg) == -1) {
    error("wron msg, msg fd:%d.", msg.fd);
  }
  free(msg.content);
  return 0;
}
