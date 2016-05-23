#include <zmq.h>
#include <stdio.h>  
#include <unistd.h>  
#include <string.h>  
#include <assert.h>
#include <sys/uio.h>
#include <curses.h>

static const char first_frame_data[] = "status";
static const char second_frame_data[] = "connected";
static const char third_frame_data[] = "0x7f,0x10,0x01,0x01,0x10,0x19";

static const char first_frame_data1[] = "status";
static const char second_frame_data1[] = "closed";
static const char third_frame_data1[] = "0x7f,0x10,0x01,0x01,0x10,0x19";

static const char first_data[] = "setting";
static const char second_data[] = "gidmap";
static const char third_data[] = "0x7f,0x10,0x01,0x01,0x10,0x19";
static const char furth_data[] = "gidaa,gidbb,gidcc";

static const char first_data2[] = "setting";
static const char second_data2[] = "gidmap";
static const char third_data2[] = "0x7f,0x10,0x01,0x01,0x10,0x19";
static const char furth_data2[] = "giddd,gidee,gidff";

static const char first_data1[] = "setting";
static const char second_data1[] = "uidmap";
static const char third_data1[] = "0x7f,0x10,0x01,0x01,0x10,0x19";
static const char furth_data1[] = "uidxxxx";

int SendLoginData(const char *firstframedata, int firLen, const char *secondframedata, int secLen, const char *thirdframedata, int thiLen)  
{
  printf("len : %d %d %d\n", firLen, secLen, thiLen);
  printf("%s\n", firstframedata);
  printf("%s\n", secondframedata);
  printf("%s\n", thirdframedata);

  zmq_msg_t part1;
  zmq_msg_t part2;
  zmq_msg_t part3;

  int rc = zmq_msg_init_size(&part1, firLen); 
  assert (rc == 0);
  rc = zmq_msg_init_size(&part2, secLen); 
  assert (rc == 0);
  rc = zmq_msg_init_size(&part3, thiLen); 
  assert (rc == 0);

  memcpy (zmq_msg_data(&part1), firstframedata, firLen);
  memcpy (zmq_msg_data(&part2), secondframedata, secLen);
  memcpy (zmq_msg_data(&part3), thirdframedata, thiLen);

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

int SendAppServerData(const char *firstdata, int firLen, const char *seconddata, int secLen, const char *thirddata, int thiLen, const char *furthdata, int furLen)
{
  printf("len : %d %d %d %d\n", firLen, secLen, thiLen, furLen);
  printf("%s\n", firstdata);
  printf("%s\n", seconddata);
  printf("%s\n", thirddata);
  printf("%s\n", furthdata);

  zmq_msg_t part1;
  zmq_msg_t part2;
  zmq_msg_t part3;
  zmq_msg_t part4;  

  int rc = zmq_msg_init_size(&part1, firLen);
  assert (rc == 0);
  rc = zmq_msg_init_size (&part2, secLen);
  assert (rc == 0);
  rc = zmq_msg_init_size (&part3, thiLen);
  assert (rc == 0);
  rc = zmq_msg_init_size (&part4, furLen);
  assert (rc == 0);

  memcpy (zmq_msg_data(&part1), firstdata, firLen);
  memcpy (zmq_msg_data(&part2), seconddata, secLen);
  memcpy (zmq_msg_data(&part3), thirddata, thiLen);
  memcpy (zmq_msg_data(&part4), furthdata, furLen);

  //Socket to talk to clients
  void *context_send = zmq_ctx_new ();
  void *sendHandle = zmq_socket(context_send, ZMQ_PUSH);
  rc = zmq_connect(sendHandle, "tcp://127.0.0.1:5558");
  assert(rc == 0);
  
  rc = zmq_sendmsg(sendHandle, &part1, ZMQ_SNDMORE);
  rc = zmq_sendmsg(sendHandle, &part2, ZMQ_SNDMORE);
  rc = zmq_sendmsg(sendHandle, &part3, ZMQ_SNDMORE);
  rc = zmq_sendmsg(sendHandle, &part4, 0);

  sleep(2);
  return 0;
}

int main(void)
{
  SendLoginData(first_frame_data, sizeof(first_frame_data)-1, second_frame_data, sizeof(second_frame_data)-1, third_frame_data, sizeof(third_frame_data)-1);
  SendAppServerData(first_data1, sizeof(first_data1)-1, second_data1, sizeof(second_data1)-1,third_data1, sizeof(third_data1)-1,furth_data1, sizeof(furth_data1)-1);
  SendAppServerData(first_data, sizeof(first_data)-1, second_data, sizeof(second_data)-1,third_data, sizeof(third_data)-1,furth_data, sizeof(furth_data)-1);
  SendAppServerData(first_data2, sizeof(first_data2)-1, second_data2, sizeof(second_data2)-1,third_data2, sizeof(third_data2)-1,furth_data2, sizeof(furth_data2)-1);
  SendLoginData(first_frame_data1, sizeof(first_frame_data1)-1, second_frame_data1, sizeof(second_frame_data1)-1, third_frame_data1, sizeof(third_frame_data1)-1);
}
