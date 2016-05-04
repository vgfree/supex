#include "pack.h"

#define LEN_PACKAGE_SIZE (sizeof(struct mfptp_pack))

#define LEN_FRAME_SIZE   (sizeof(struct mfptp_frame))

#define MFPTP_FRAME_MAX_LEN (1024*1024*6)

#define PTR_MOVE(len) do {      \
           offset += len;       \
 }while(0)




int mfptp_pack_len(int len)
{
     return len +128 ; 
}

struct mfptp_pack *copy_list(struct mfptp_pack *head,struct mfptp_pack *cop,int len)
{
    struct mfptp_pack *p1,*p2,*p;
    p1=head;
    p=(struct mfptp_pack*)malloc(len) ; /*p用于指向新开辟的节点*/
    if(p == NULL){
        return NULL;
    }
    p2=cop=p;
    while(p1->next!=NULL)
    {
        memcpy(p2,p1,len);
        p1=p1->next;

        p = (struct mfptp_pack*)malloc(len);
        if(p == NULL){
            return NULL;
        }
        p2->next=p;
        p2=p2->next;
    }
    memcpy(p2,p1,len);
    p2->next=NULL;
    return (cop);
}


struct mfptp_frame *copy_frame_list(struct mfptp_frame *head,struct mfptp_frame *cop,int len)
{
    struct mfptp_frame *p1,*p2,*p;
    p1=head;
    p=(struct mfptp_frame*)malloc(len) ; /*p用于指向新开辟的节点*/
    if(p == NULL){
        return NULL;
    }
    p2=cop=p;
    while(p1->next!=NULL)
    {
        memcpy(p2,p1,len);
        p1=p1->next;

        p = (struct mfptp_frame*)malloc(len);
        if(p == NULL){
            return NULL;
        }
        p2->next=p;
        p2=p2->next;
    }
    memcpy(p2,p1,len);
    p2->next=NULL;
    return (cop);
}

struct mfptp_pack * pack_one_package(struct mfptp_pack * p_info)
{
    short mfptp_version;
    unsigned char mfptp_encry_compress;
    int package_size = 0;
    struct mfptp_frame *ss = p_info->m_frame; 
    while(ss){
          package_size    += ss->size;
          ss               = ss->next;
    }
    int pack_len  = mfptp_pack_len(package_size);
    printf("pack_len = %d\n",pack_len);
    p_info->pack_buf =(char *)malloc(pack_len);
    if(p_info->pack_buf == NULL){
          printf("malloc error\n");
          return NULL;
    }
    /* 计算加密压缩控制位  */
    unsigned char  p_compress = p_info->compress << 4;
    p_compress               |= 0x0f;
    mfptp_encry_compress      = p_info->encrypt | 0xf0;
    p_compress               &= mfptp_encry_compress;
    /* 计算版本控制位 */
    unsigned char p_version   = ((unsigned char )p_info->version) << 4;
    p_version                |= 0x0f;
    mfptp_version             = p_info->sub_version |0xf0;
    p_version                &= (unsigned char)mfptp_version;
    int offset = 0;
    char * dst = p_info->pack_buf;
    /* 包头前六位必须是#MFPTP */
    memcpy(dst,"#MFPTP",6);
    PTR_MOVE(6);
    /* 版本  */
    *(dst+offset) =p_version;
    PTR_MOVE(1);

    /* 加密压缩  */
    *(dst+offset) =p_compress;
    PTR_MOVE(1);
    /* socket 方法  */
    *(dst+offset) =p_info->method;
    PTR_MOVE(1);

    /* 包的个数  */
    *(dst+offset) =p_info->packages;
    PTR_MOVE(1);
    
    int de_len;
    int out_len ;
    char  *buf  = NULL;
    char  *buf1 = NULL;
    if(1){
              buf1 = malloc(MFPTP_FRAME_MAX_LEN);
              if(NULL == buf1){

                   printf("加密内存分配错误\n");
                   return NULL;
              }
   }

   if(1){
              buf = malloc(MFPTP_FRAME_MAX_LEN);
              if(NULL == buf){

                   printf("压缩内存分配错误\n");
                   free(buf1);
                   return NULL;
               }
   }

   while(p_info->m_frame){
          if(p_info->compress == 0x02){

                /*gzip compress operation   */

          }else if(p_info->compress == 0x01){

                /*zip  compress operation   */

          }else{
                de_len = p_info->m_frame->size;
                memcpy(buf,p_info->m_frame->frame_buf,p_info->m_frame->size);
          }
          
          if(p_info->encrypt == 0x02){

                /*aes encrypt operation */

          }else if(p_info->encrypt == 0x01){

                /*idea encrypt operation */

          }else{

                out_len = de_len;
                memcpy(buf1,buf,de_len);
          }
    
          if(p_info->m_frame->next == NULL){

              unsigned char *p=(unsigned char *)&out_len;
              if(out_len < 255){
                  *(dst+offset) = 0x00;
                  PTR_MOVE(1);
                  *(dst+offset) = out_len;
                  PTR_MOVE(1);
                  memcpy(dst+offset,buf1,out_len);
                  PTR_MOVE(out_len);
              }else if(out_len <= 65535){
                  *(dst+offset) = 0x01; /* 设置FP_CONTROL  */
                  PTR_MOVE(1);
                  *(dst+offset) =*(p+1);
                  PTR_MOVE(1);
                  *(dst+offset) =*(p);
                  PTR_MOVE(1);
                  memcpy(dst+offset,buf1,out_len);
                  PTR_MOVE(out_len);   
              }else{
                  *(dst+offset) = 0x02; /* 设置FP_CONTROL  */
                  PTR_MOVE(1);
                  *(dst+offset) =*(p+2);
                  PTR_MOVE(1);
                  *(dst+offset) =*(p+1);
                  PTR_MOVE(1);
                  *(dst+offset) =*(p);
                  PTR_MOVE(1);
                  memcpy(dst+offset,buf1,out_len);
                  PTR_MOVE(out_len);
              }
          }else{
              unsigned char *p=(unsigned char *)&out_len;
              if(out_len < 255){
                  *(dst+offset) = 0x04;
                  PTR_MOVE(1);
                  *(dst+offset) = out_len;
                  PTR_MOVE(1);
                  memcpy(dst+offset,buf1,out_len);
                  PTR_MOVE(out_len);
              }else if(out_len <= 65535){
                  *(dst+offset) = 0x04;
                  *(dst+offset) =(*(dst+offset)) |0x01; /* 设置FP_CONTROL  */
                  PTR_MOVE(1);
                  *(dst+offset) =*(p+1);
                  PTR_MOVE(1);
                  *(dst+offset) =*(p);
                  PTR_MOVE(1);
                  memcpy(dst+offset,buf1,out_len);
                  PTR_MOVE(out_len);
              }else{
                  *(dst+offset) = 0x04;
                  *(dst+offset) = (*(dst+offset)) |0x02; /* 设置FP_CONTROL  */
                  PTR_MOVE(1);
                  *(dst+offset) =*(p+2);
                  PTR_MOVE(1);
                  *(dst+offset) =*(p+1);
                  PTR_MOVE(1);
                  *(dst+offset) =*(p);
                  PTR_MOVE(1);
                  memcpy(dst+offset,buf1,out_len);
                  PTR_MOVE(out_len);
              }
          }
          p_info->m_frame = p_info->m_frame->next;
      }
      free(buf);
      free(buf1);
      return p_info; 
}

struct mfptp_pack *  mfptp_packages(struct mfptp_pack * p_info)
{
    assert(p_info);
    assert(p_info->m_frame);
    struct mfptp_pack *src,*p_src;
    src = copy_list(p_info,src,LEN_PACKAGE_SIZE);
    if(src == NULL){
         return NULL;
    }
    p_src = src;
    struct mfptp_frame *frame_src,*p_frame_src;
    while(p_src){
         frame_src =copy_frame_list(p_src->m_frame,frame_src,LEN_FRAME_SIZE);
         if(frame_src == NULL){
                return NULL;
         }
         p_frame_src = frame_src;
         p_src->m_frame =frame_src;
         p_src =  pack_one_package(p_src);
         if(p_src == NULL){
                return NULL;
         }
         p_src = p_src->next;
    }    
         return src;
}
