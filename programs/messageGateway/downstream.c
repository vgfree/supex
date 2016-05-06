#include "comm_io_wraper.h"
#include "downstream.h"

int downstream_msg(struct comm_message *msg)
{
  for (int i = 0; i < g_node_ptr->max_size; i++) {
    msg->fd = g_node_ptr->fd_array[i];
    send_msg(msg);
  } 
  return 0;
}
