/**
 * g++ pub-sub-dynamic-discover-publisher.cpp -lzmq -g -O0 -o objs/pub-sub-dynamic-discover-publisher
 * */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zmq.h>
#define zmq_print_error()    printf("%s[%d]: %d: %s\n", __FILE__, __LINE__, \
                        zmq_errno(), zmq_strerror(zmq_errno()));
int main(int argc, char** argv){
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    int version = ZMQ_MAKE_VERSION(major, minor, patch);
    int sendlen = 0;
    printf("argc = %d\n",argc);
    if (1)
    {
        printf("zmq library expect version %d but get %d.\n", ZMQ_VERSION, version);
    }

    void* context = zmq_ctx_new();
    void* publisher = zmq_socket(context, ZMQ_PUSH);
    int r = zmq_bind(publisher, "tcp://*:19991");
    assert(r==0);
    
    srand(0);
    const char* filter = (argc > 1)? argv[1] : "abc";
    printf("start to publish, filter=%s\n", filter);
    
    int i=0;
    int count = 3;
    char *buf=(char *)malloc(1024*1024*6);
    while(count --){
        //snprintf(buf, sizeof(buf), "%s","1234567890");
        sprintf(buf, "%s","1234567890");
        buf[10]=0;
        printf("publish message: %s\n", buf);
        
        zmq_msg_t name;
        zmq_msg_init_size(&name, strlen(buf)+1);
        memcpy(zmq_msg_data(&name), buf, strlen(buf)+1);
        zmq_msg_send(&name, publisher, ZMQ_SNDMORE);
        zmq_msg_close(&name);
        int frame_count = 1;
        int snd;
        while(frame_count--){
              if(0 == frame_count){
                 snd = 0;
              }else{
                 snd = ZMQ_SNDMORE;
              }
              sprintf(buf, "%s i=%d %s j=%d k=%d", filter, rand(),"abc", rand(), rand());
              int len = strlen(buf)+1;
              len = 1024*1024;
              if(argc==2){
                      sendlen = atoi(argv[1]);
                      len = sendlen;
              }
              zmq_msg_t msg;
              printf("publish message:%d %s\n", len,buf);
   
              if(zmq_msg_init_size(&msg, len)<0){
                      zmq_print_error();
              }
              //char * s = zmq_msg_data(&msg);
              //memcpy(s, buf, len);
              zmq_msg_send(&msg, publisher,snd);
              zmq_msg_close(&msg);
        
        }
        //usleep( 10*1000);
    }
    
    zmq_close(publisher);
    zmq_ctx_destroy(context);
    return 0;
}
