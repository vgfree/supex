#include "comm_io_wraper.h"
#include "comm_message_operator.h"
#include "loger.h"
#include "upstream.h"
#include "zmq_io_wraper.h"

int upstream_msg()
{
  struct comm_message msg = {};
  msg.content = (char *)malloc(102400 * sizeof(char));
  recv_msg(&msg);
  int fsz = 0;
  char *destination = get_msg_frame(0, &msg, &fsz);
  if (memcmp(destination, "cidServer", 9) == 0) {
    log("It's cid server."); 
  }
  else {
    char frame[30] = {};
    memcpy(frame, destination, fsz);
    error("not support this server:%s.", frame);
    return -1;
  }
  remove_first_nframe(1, &msg);
  for (int i = 0; i < get_max_msg_frame(&msg); i++) {
    zmq_msg_t msg_frame;
    int fsz = 0;
    char *frame = get_msg_frame(i, &msg, &fsz);
    int rc = zmq_msg_init_size(&msg_frame, fsz);
    assert(rc == 0);
    memcpy(zmq_msg_data(&msg_frame), frame, fsz);
    if (i < get_max_msg_frame(&msg) - 1) {
      zmq_io_send(CID_SERVER, &msg_frame, ZMQ_SNDMORE);
    }
    else {
      zmq_io_send(CID_SERVER, &msg_frame, 0);
    }
  } 
  free(msg.content);
  return 0;
}
