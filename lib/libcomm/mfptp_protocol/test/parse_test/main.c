#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parse.h"


int main()
{
  int len  = 0;
  char buf[100];
  char * p = buf;
  memcpy(p,"#MFPTP",6);
  len  =  len +6;
  buf[len] =  0x00;
  buf[len+1] = 0x00;
  buf[len+2] = 0x00;
  buf[len+3] = 0x01;
  len = len +4;
  printf("len = %d \n",len);
 
  buf[len] = 0x04;
  buf[len+1] = 0x08;
  len = len +2;
  memcpy(p+len,"chendong",8);
  len = len +8;

  buf[len] = 0x00;
  buf[len+1] = 0x07;
  len = len +2;
  memcpy(p+len,"chenxue",7);
  len = len +7;

  memcpy(p+len,"#MFPTP",6);
  len = len +6;

  buf[len] =  0x00;
  buf[len+1] = 0x00;
  buf[len+2] = 0x00;
  buf[len+3] = 0x01;
  len = len +4;
  printf("len = %d \n",len);

  buf[len] = 0x00;
  buf[len+1] = 0x0A;
  len = len +2;
  memcpy(p+len,"chenliuliu",10);
  len = len +10;
  printf("len = %d \n",len);

  memcpy(p+len,"#MFPTP",6);
  len = len +6;
  
  buf[len] =  0x00;
  buf[len+1] = 0x00;
  buf[len+2] = 0x00;
  buf[len+3] = 0x01;
  len = len +4;

  buf[len] = 0x00;
  buf[len+1] = 0x0A;
  len = len +2;
  memcpy(p+len,"chen",4);
  len = len +4;
  printf("len = %d \n",len);
  struct  mfptp_parse * p_info = NULL;
  p_info = parse_data(len,buf,p_info); 
  if(p_info == NULL){
        printf("------- NULL --------\n");
  }else{
        printf("------- node --------\n");
        while(p_info){
             printf("%d\n",p_info->off_size);
             while(p_info->m_frame){
                   printf("@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
                   printf("p_info->m_frame->size = %d \n",p_info->m_frame->size);
                   printf("buf = %s\n",p_info->m_frame->frame_buf);
                   p_info->m_frame =  p_info->m_frame->next;  
             }
             p_info = p_info->next;
        }
  }

/*
  int a = judge_package_all(51,27,buf);
  printf("a = %d\n",a);
*/

  return 0;
}
