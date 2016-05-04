#include "comm_io_wraper.h"
#include "router.h"
#include "upstream.h"
#include "zmq_io_wraper.h"

int upstream_msg()
{
  struct comm_message msg;
  msg.content = (char *)malloc(102400 * sizeof(char));
  recv_msg(&msg);
  int sz = 0;
  if (msg.frames < 1) {
    printf("wrong msg, msg frames:%d.", msg.frames);
    return -1;
  }
  else if (msg.frames == 1) {
    sz = msg.dsize;
  }
  else {
    sz = frame_offset[1] - frame_offset[0];
  }
  struct router_head *hd = parse_router(msg.content, sz);
  assert(hd);
  for (int i = 0; i < msg.frames; i++) {
    zmq_msg_t msg_frame;
    int rc = zmq_msg_init_size(&msg_frame, msg.frame_offset[i + 1] - msg.frame_offset[i]);
    assert(rc == 0);
    memcpy(zmq_msg_data(&msg_frame), msg.content + msg.frame_offset[i],
           msg.frame_offset[i + 1] - msg.frame_offset[i]);
	if (i < msg.frames - 1) {
      zmq_send(hd->message_to, &msg_frame, ZMQ_SNDMORE);
    }
	else {
      zmq_send(hd->message_to, &msg_frame, 0);
	}
  } 
}
