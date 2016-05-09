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


static const char first_frame_data[] = "feedback?imei=177238787196594";

static const char second_frame_data[] = "json\r\n{\"ERRORCODE\":\"0\",\"RESULT\":{\"accountID\":\"BM9xkNDQlh\",\"content\":{\"altitude\":17,\"direction\":86,\"latitude\":31.202028333333335,\"longitude\":121.69391833333333,\"mediaList\":[{\"format\":\"jpg\",\"index\":\"3\",\"pixels\":\"1920*1080\",\"size\":\"323153\"}],\"speed\":69},\"imei\":\"144601531989913\",\"imsi\":\"460060001040067\",\"mod\":\"XZ001\",\"operationType\":\"3\",\"remarkMsg\":\"............\",\"type\":\"20\"}}";

static const char third_pic_head[] = "jpg\r\n";

void recive_some_data(struct skt_device *devc)
{
  size_t  count = MAX_SPILL_DEPTH;
  int rc = zmq_recviov(devc->skt, devc->ibuffer, &count, 0);
  printf("after zmq_recv, count = %d\n", count);
  devc->idx = count;
}


int main (void)  
{
  /////////read picture
  FILE *pFile = fopen("Craire.jpeg", "rb");                                 
  fseek(pFile ,0 ,SEEK_END); //把指针移动到文件的结尾 ，获取文件长度          
  int plen=ftell(pFile); //获取文件长度                                        
  printf("picture len = %d\n", plen);                                                  
  char pBuf[plen+sizeof(third_pic_head)];                                                         
  rewind(pFile); //把指针移动到文件开头 因为我们一开始把指针移动到结尾，如果不移动回来 会出错
  memcpy(pBuf, third_pic_head, sizeof(third_pic_head));
  fread(pBuf+sizeof(third_pic_head), 1, plen, pFile); //读文件                                        
  pBuf[plen+sizeof(third_pic_head)]=0; //把读到的文件最后一位 写为0 要不然系统会一直寻找到0后才结束   
  fclose(pFile); // 关闭文件
  ///////////  read audio
//  FILE *aFile = fopen("testsound.amr", "rb");   
//  fseek(aFile ,0 ,SEEK_END); //把指针移动到文件的结尾 ，获取文件长度          
//  int alen=ftell(pFile); //获取文件长度                                        
//  printf("audio len = %d\n", alen);                                               
//  char aBuf[alen];                                                              
//  rewind(aFile); //把指针移动到文件开头 因为我们一开始把指针移动到结尾，如果不移动回来 会出错
//  fread(aBuf, 1, alen, aFile); //读文件                                        
//  pBuf[alen]=0; //把读到的文件最后一位 写为0 要不然系统会一直寻找到0后才结束   
//  fclose(aFile); // 关闭文件
  //////////////////end 

  zmq_msg_t part1;
  zmq_msg_t part2;
  zmq_msg_t part3;
//  zmq_msg_t part4;

  int rc = zmq_msg_init_size(&part1, sizeof(first_frame_data)); 
  assert (rc == 0);
  rc = zmq_msg_init_size (&part2, sizeof(second_frame_data)); 
  assert (rc == 0);
  rc = zmq_msg_init_size (&part3, sizeof(pBuf)); 
  assert (rc == 0);
//  rc = zmq_msg_init_size (&part4, sizeof(aBuf));
//  assert (rc == 0);

  memcpy (zmq_msg_data(&part1), first_frame_data, sizeof(first_frame_data));
  memcpy (zmq_msg_data(&part2), second_frame_data, sizeof(second_frame_data));
  memcpy (zmq_msg_data(&part3), pBuf, sizeof(pBuf));
//  memcpy (zmq_msg_data(&part4), aBuf, sizeof(aBuf));

  //Socket to talk to clients
  struct skt_device devc = {};
 
  void *context_send = zmq_ctx_new ();
  void *sendHandle = zmq_socket(context_send, ZMQ_PUSH);
  rc = zmq_bind(sendHandle, "tcp://*:5556");
  assert(rc == 0);
  
  void *context_recv = zmq_ctx_new ();
  devc.skt = zmq_socket(context_recv, ZMQ_PULL);
  rc = zmq_connect(devc.skt, "tcp://127.0.0.1:1020");
  assert(rc == 0);

  rc = zmq_sendmsg(sendHandle, &part1, ZMQ_SNDMORE);
  rc = zmq_sendmsg(sendHandle, &part2, ZMQ_SNDMORE);
//  rc = zmq_sendmsg(sendHandle, &part3, ZMQ_SNDMORE);
  rc = zmq_sendmsg(sendHandle, &part3, 0);
  
  recive_some_data(&devc);

  sleep(2);
  return 0;  
}
