#include <zmq.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <string.h>  
#include <assert.h>  

static const char first_frame_data[] = "feedback?imei=123456789123456";
static const char second_frame_data[] = "{\"ERRORCODE\"\:\"0\",\"RESULT\":{\"accountID\":\"uRgZGyPykT\",\"content\":{\"mediaList\":[{\"format\":\"jpg\",\"index\":\"3\",\"pixels\":\"1920*1080\",\"size\":\"318215\"}]},\"imei\":\"306627488190175\",\"imsi\":\"460017204705594\",\"mod\":\"XZ001\",\"operationType\":\"3\",\"remarkMsg\":\"拍摄成功\",\"type\":\"20\"}}"; 

int main (void)  
{
  /////////read picture
  FILE *pFile = fopen("Craire.jpeg", "rb");                                 
  fseek(pFile ,0 ,SEEK_END); //把指针移动到文件的结尾 ，获取文件长度          
  int plen=ftell(pFile); //获取文件长度                                        
  printf("picture len = %d\n", plen);                                                  
  char pBuf[plen + 1];                                                         
  rewind(pFile); //把指针移动到文件开头 因为我们一开始把指针移动到结尾，如果不移动回来 会出错
  fread(pBuf, 1, plen, pFile); //读文件                                        
  pBuf[plen]=0; //把读到的文件最后一位 写为0 要不然系统会一直寻找到0后才结束   
  fclose(pFile); // 关闭文件
  ///////////  read audio
  FILE *aFile = fopen("testsound.amr", "rb");   
  fseek(aFile ,0 ,SEEK_END); //把指针移动到文件的结尾 ，获取文件长度          
  int alen=ftell(pFile); //获取文件长度                                        
  printf("audio len = %d\n", alen);                                               
  char aBuf[alen + 1];                                                              
  rewind(aFile); //把指针移动到文件开头 因为我们一开始把指针移动到结尾，如果不移动回来 会出错
  fread(aBuf, 1, alen, aFile); //读文件                                        
  pBuf[alen]=0; //把读到的文件最后一位 写为0 要不然系统会一直寻找到0后才结束   
  fclose(aFile); // 关闭文件
  //////////////////end 

  zmq_msg_t part1;
  zmq_msg_t part2;
  zmq_msg_t part3;
  zmq_msg_t part4;

  int rc = zmq_msg_init_size(&part1, sizeof(first_frame_data)); 
  assert (rc == 0);
  rc = zmq_msg_init_size (&part2, sizeof(second_frame_data)); 
  assert (rc == 0);
  rc = zmq_msg_init_size (&part3, sizeof(pBuf)); 
  assert (rc == 0);
  rc = zmq_msg_init_size (&part4, sizeof(aBuf));
  assert (rc == 0);

  memcpy (zmq_msg_data(&part1), first_frame_data, sizeof(first_frame_data));
  memcpy (zmq_msg_data(&part2), second_frame_data, sizeof(second_frame_data));
  memcpy (zmq_msg_data(&part3), pBuf, sizeof(pBuf));
  memcpy (zmq_msg_data(&part4), aBuf, sizeof(aBuf));

  //Socket to talk to clients 
  void *context = zmq_ctx_new ();
  void *responder = zmq_socket (context, ZMQ_PUSH);
  rc = zmq_bind (responder, "tcp://*:5556");
  assert (rc == 0);

  rc = zmq_sendmsg (responder, &part1, ZMQ_SNDMORE);
  //rc = zmq_sendmsg (responder, &part2, 0);
  rc = zmq_sendmsg (responder, &part2, ZMQ_SNDMORE);
  rc = zmq_sendmsg (responder, &part3, ZMQ_SNDMORE);
  rc = zmq_sendmsg (responder, &part4, 0);

  sleep(2);
  return 0;  
}
