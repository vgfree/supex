#include <zmq.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <string.h>  
#include <assert.h>  

static const char data[512] = "keyvalue\r\nnt=0&mt=21776&Accountid=111111111111111&gps=221215102957,11444.98295E,4045.92141N,262,18,746;221215102958,11444.97898E,4045.92095N,262,19,746;221215102959,11444.97479E,4045.92069N,263,20,746;221215103000,11444.97046E,4045.92029N,262,20,746;221215103001,11444.96605E,4045.91999N,262,21,746&tokencode=0al15UMqpo&imsi=460022211427272&imei=752795632561713&mod=SG900";

int main (void)  
{  
  //Socket to talk to clients 
  void *context = zmq_ctx_new ();
  void *responder = zmq_socket (context, ZMQ_PUSH);
  int rc = zmq_bind (responder, "tcp://*:5555");
  assert (rc == 0);
  char buffer [10];  
  char *send_s = "World";  
    // 接收消息  
    //zmq_recv(responder, buffer, 10, 0);
    //buffer[5] = 0;
    //printf ("Recv %s\n", buffer);  
    // 发送反馈  
    zmq_send (responder, buffer, 10, 0);  
    printf ("Send %s\n", buffer);  
    sleep(1);
  return 0;  
}
