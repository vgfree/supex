#include "communication.h"
#include "comm_io_wraper.h"
#include "router.h"
#include "validate_logon.h"

int send_validate_logon_package(int fd)
{
  struct router_head head = {};
  head.message_from = MESSAGE_GATEWAY;
  head.message_to = ROUTER_SERVER;
  head.type = 0x04;
  head.body_size = 0x00;
  struct comm_message msg = {};
  msg.fd = fd;
  struct comm_message *sendmsg = pack_router(&head, &msg);
  send_msg(sendmsg);
  free(sendmsg->content);
  free(sendmsg);
  return 0;
}
