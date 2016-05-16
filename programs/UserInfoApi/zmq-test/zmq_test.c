#include <zmq.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <string.h>  
#include <assert.h>
#include <sys/uio.h>

static const char first_frame_data[] = "status";
static const char second_frame_data_connect[] = "connected";
static const char third_frame_data[] = "xxxxxx";

int main (void)  
{
  zmq_msg_t part1;
  zmq_msg_t part2;
  zmq_msg_t part3;

  int rc = zmq_msg_init_size(&part1, sizeof(first_frame_data)); 
  assert (rc == 0);
  rc = zmq_msg_init_size (&part2, sizeof(second_frame_data_connect)); 
  assert (rc == 0);
  rc = zmq_msg_init_size (&part3, sizeof(third_frame_data)); 
  assert (rc == 0);

  memcpy (zmq_msg_data(&part1), first_frame_data, sizeof(first_frame_data));
  memcpy (zmq_msg_data(&part2), second_frame_data_connect, sizeof(second_frame_data_connect));
  memcpy (zmq_msg_data(&part3), third_frame_data, sizeof(third_frame_data));

  //Socket to talk to clients
  void *context_send = zmq_ctx_new ();
  void *sendHandle = zmq_socket(context_send, ZMQ_PUSH);
  rc = zmq_connect(sendHandle, "tcp://127.0.0.1:5558");
  assert(rc == 0);
  
  rc = zmq_sendmsg(sendHandle, &part1, ZMQ_SNDMORE);
  rc = zmq_sendmsg(sendHandle, &part2, ZMQ_SNDMORE);
  rc = zmq_sendmsg(sendHandle, &part3, 0);

  sleep(2);
  return 0;  
}
