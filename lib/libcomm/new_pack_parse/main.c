#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pack.h"
#include "parse.h"


int main()
{
  char buf1[20] ="chenliuliu";
  char buf2[20] ="dongfeng";
  char buf3[20] ="xifeng";
  char buf4[20] ="nanfeng";

  struct mfptp_pack *root, head, first;
  struct mfptp_pack *ss,*dd,*ll;
  root = &head;
  ss = root;
  head.method = 0x02;
  head.compress = 0x00;
  head.encrypt  = 0x00;
  head.version  = 0x00;
  head.sub_version = 0x01;
  head.packages = 0x01;
  struct mfptp_frame *root1 ,head1,first1;
  root1 = &head1;
  head1.size = 10; 
  head1.frame_buf = buf1;
  head1.next = &first1;
  
  first1.size = 8; 
  first1.frame_buf = buf2;
  first1.next = NULL;
  
  head.m_frame = root1;

/*------------------------*/
 

  root->next = &first;  
  
  first.method = 0x02;
  first.compress = 0x00;
  first.encrypt  = 0x00;
  first.version  = 0x00;
  first.sub_version = 0x02;
  first.packages = 0x01;
  struct mfptp_frame *root2 ,head2,first2;
  root2 = &head2;
  head2.size = 6; 
  head2.frame_buf = buf3;
  head2.next = &first2;
  
  first2.size = 9; 
  first2.frame_buf = buf4;
  first2.next = NULL;

  
  first.next = NULL; 

  first.m_frame = root2;

//  printf("%d\n",root->m_frame->size);
  if(ss ==NULL){
       printf("ssssssssssssssss\n");
  }
  while(ss){
             printf("%s\n",ss->m_frame->frame_buf);
             ss = ss->next;
  }
  root = mfptp_packages(root);
  int i = 0 ;
  dd =root;
  ll =root;
 
  for(i=0;i<32;i++){

     if(i>=0 && i<=11){
          printf("%d ",dd->pack_buf[i]);
     }else if(i>=22 && i<=23){
          printf("%d ",dd->pack_buf[i]);
     }else{
          printf("%c ",dd->pack_buf[i]);
     }

  }
  printf("\n");

//======================================
  char  buf_pack[200]; 
  int   size_pack = 0;
  while(dd){
      memcpy(buf_pack+size_pack,dd->pack_buf,dd->pack_buf_size);
      size_pack += dd->pack_buf_size; 
      printf("size_pack = %d\n",size_pack);
      dd = dd->next;
  }  
  struct mfptp_parse *p_info = NULL;
  p_info = parse_data(size_pack,buf_pack,p_info);
  
  if(p_info == NULL){
         printf("------- NULL --------\n");
   }else{
         printf("------- node --------\n");
         while(p_info){
              printf("off_size =%d\n",p_info->off_size);
              while(p_info->m_frame){
                    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
                    printf("p_info->m_frame->size = %d \n",p_info->m_frame->size);
                    printf("buf = %s\n",p_info->m_frame->frame_buf);
                    p_info->m_frame =  p_info->m_frame->next;
              }
              p_info = p_info->next;
         }
   }




  return 0;
}
