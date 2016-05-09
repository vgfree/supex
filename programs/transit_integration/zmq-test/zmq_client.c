#include <zmq.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <string.h>  
#include <assert.h>
#include <sys/uio.h>

#define MAX_SPILL_DEPTH 32

struct skt_device
{
  int             idx;
  void            *skt;
  struct iovec    ibuffer[MAX_SPILL_DEPTH];
};

void recive_some_data(struct skt_device *devc)
{
	size_t  count = MAX_SPILL_DEPTH;
	int rc = zmq_recviov(devc->skt, devc->ibuffer, &count, 0);
	printf("after zmq_recv, count = %d %d\n", count, rc);
	devc->idx = count;
}


int main (void)  
{

  //Socket to talk to clients
  struct skt_device devc = {};
 
  void *context = zmq_ctx_new ();
  void *handle = zmq_socket(context, ZMQ_PULL);
 devc.skt = handle;
  int rc = zmq_connect(handle, "tcp://127.0.0.1:1020");
  assert(rc == 0);

  while(1) {
	  recive_some_data(&devc);
  }
  return 0;  
}
