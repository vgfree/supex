#include "parse.h"


#define FP_CONTROL_LOW01_BITS_MASK      (0x03)
#define FP_CONTROL_LOW23_BITS_MASK      (0x0C)

#define GET_LOW01_BITS(ctrl)    (ctrl & FP_CONTROL_LOW01_BITS_MASK)
#define GET_LOW23_BITS(ctrl)    ((ctrl & FP_CONTROL_LOW23_BITS_MASK) >> 2)

#define MFPTP_FRAME_MAX_LEN (1024*1024*6)

#define PTR_MOVE(len) do {      \
           offset += len;       \
 }while(0)


struct  mfptp_parse *  parse_data(int size,char *buf,struct mfptp_parse *p_info)
{
     assert(buf);
     if(size < 12){
           return NULL;
     }
     int  flag       = 1;
     int  i          = 0;
     char * ptr      = buf;
     int offset      = 0; 
     int frame_size  = 0;
     int size_f_size = 0;
     short version,sub_version;
     unsigned char compress,encrypt,method,packages;
     unsigned char fp_control;
     struct mfptp_parse * p_dst = (struct mfptp_parse *)malloc(sizeof(struct mfptp_parse));
     if(p_dst == NULL){
           return NULL;
     }
     p_info  = p_dst;
     p_dst->next = NULL;
     p_dst->m_frame = NULL;
     p_dst->off_size = 0;
     struct mfptp_parse *package_ptr = NULL;
     struct mfptp_frame *frame_ptr = NULL;
     struct mfptp_frame *src_frame = NULL;
     int package_all ;
     while(size - offset > 12){
         package_all = judge_package_all(size,offset,ptr);    
         if(package_all == 1){  

             /* 包的前6个字节必须是"#MFPTP" */
             if (strncmp(ptr + offset, "#MFPTP", 6) != 0) { 
                    free_malloc(p_info);
                    return NULL;
             }
             PTR_MOVE(6);
             /* 版本信息 */
             version     = (unsigned char)(*(ptr + offset) >> 4);
             sub_version = (unsigned char)(*(ptr + offset) << 4);
             sub_version = sub_version >> 4;
             PTR_MOVE(1);

             /* 加密和压缩信息 */
             compress    = (unsigned char)(*(ptr + offset) >> 4);
             encrypt     = (unsigned char)(*(ptr + offset) << 4);
             encrypt     = encrypt >> 4;
             PTR_MOVE(1);

             /* 方法信息 */
             method      = (unsigned char)(*(ptr + offset));
             PTR_MOVE(1);

             /* 包数信息 */
             packages = (unsigned char)(*(ptr + offset));
             PTR_MOVE(1);
             printf("offset = %d \n",offset);
             if(package_ptr !=NULL){

                    package_ptr = (struct mfptp_parse *)malloc(sizeof(struct mfptp_parse));
                    if(package_ptr == NULL){
                             free_malloc(p_info);
                             return NULL;
                    }
                    package_ptr->off_size = 0;
                    p_dst->next = package_ptr;
                    p_dst       = package_ptr;
                    src_frame = (struct mfptp_frame *)malloc(sizeof(struct mfptp_frame));
                    if(src_frame == NULL){
                             free_malloc(p_info);
                             return NULL;
                    }
                    p_dst->m_frame = src_frame;
                    src_frame->frame_buf = NULL;
                    src_frame->size      = 0;
                    frame_ptr = NULL;
                    flag = 1;
                    while(flag){
                           printf("****************************\n");
                           /* FP_control 字段*/
                           fp_control = (unsigned char)(*(ptr + offset));

                           /* fp_control的最低2位表示f_size字段占据的长度*/
                           size_f_size = GET_LOW01_BITS(fp_control) + 1;
  
                           /* fp_control的第2、3位表示package是否结束,只有0和1两个取值*/
                           bool complete = GET_LOW23_BITS(fp_control);
 
                           /* 取出数据长度，本长度低地址字节表示高位*/
                           frame_size = 0;

                           for (i = 0; i < size_f_size; i++) {
                                    frame_size += *(ptr + i + offset + 1) << (8 * (size_f_size - 1 - i));
                           }
               
                          if(frame_ptr != NULL){
                                   frame_ptr = (struct mfptp_frame *)malloc(sizeof(struct mfptp_frame));
                                   if(frame_ptr == NULL){
                                               free_malloc(p_info);
                                               return NULL;
                                   }
                                   frame_ptr->frame_buf = NULL;
                                   src_frame->next = frame_ptr;
                                   src_frame       = frame_ptr;
                                   src_frame->frame_buf = (char *)malloc(size_f_size + 1);
                                   if(src_frame->frame_buf == NULL){
                                               free_malloc(p_info);
                                               return NULL;
                                   }
                                   memcpy(src_frame->frame_buf,ptr+offset+1+size_f_size,frame_size);
                                   src_frame->size = frame_size;
                                   PTR_MOVE((frame_size+1+size_f_size));
                                   printf("offset = %d \n",offset);
                                   flag = complete;
                          }else{
                                   src_frame->frame_buf = (char *)malloc(size_f_size + 1);
                                   if(src_frame->frame_buf == NULL){
                                               free_malloc(p_info);
                                               return NULL;
                                   }
                                   memcpy(src_frame->frame_buf,ptr+offset+1+size_f_size,frame_size);
                                   src_frame->size = frame_size;
                                   PTR_MOVE((frame_size+1+size_f_size));
                                   printf("offset = %d \n",offset);
                                   frame_ptr = src_frame;
                                   flag = complete;
                          }
                    }
             }else{
                     src_frame = (struct mfptp_frame *)malloc(sizeof(struct mfptp_frame));
                     if(src_frame == NULL){
                                               free_malloc(p_info);
                                               return NULL;
                     }
                     p_dst->m_frame = src_frame;
                     src_frame->frame_buf = NULL;
                     src_frame->size      = 0;
                     frame_ptr = NULL;
                     flag = 1;
                     while(flag){
                           /* FP_control 字段*/
                           fp_control = (unsigned char)(*(ptr + offset));

                           /* fp_control的最低2位表示f_size字段占据的长度*/
                           size_f_size = GET_LOW01_BITS(fp_control) + 1;
  
                           /* fp_control的第2、3位表示package是否结束,只有0和1两个取值*/
                           bool complete = GET_LOW23_BITS(fp_control);
 
                           /* 取出数据长度，本长度低地址字节表示高位*/
                           frame_size = 0;

                           for (i = 0; i < size_f_size; i++) {
                                    frame_size += *(ptr + i + offset + 1) << (8 * (size_f_size - 1 - i));
                           }
               
                          if(frame_ptr != NULL){
                                   frame_ptr = (struct mfptp_frame *)malloc(sizeof(struct mfptp_frame));
                                   if(frame_ptr == NULL){
                                               free_malloc(p_info);
                                               return NULL;
                                   }
                                   frame_ptr->frame_buf = NULL;
                                   src_frame->next = frame_ptr;
                                   src_frame       = frame_ptr;
                                   src_frame->frame_buf = (char *)malloc(size_f_size + 1);
                                   if(src_frame->frame_buf == NULL){
                                               free_malloc(p_info);
                                               return NULL;
                                   }
                                   memcpy(src_frame->frame_buf,ptr+offset+1+size_f_size,frame_size);
                                   src_frame->size = frame_size;
                                   PTR_MOVE((frame_size+1+size_f_size));
                                   flag = complete;
                          }else{
                                   src_frame->frame_buf = (char *)malloc(size_f_size + 1);
                                   if(src_frame->frame_buf == NULL){
                                               free_malloc(p_info);
                                               return NULL;
                                   }
                                   memcpy(src_frame->frame_buf,ptr+offset+1+size_f_size,frame_size);
                                   src_frame->size = frame_size;
                                   PTR_MOVE((frame_size+1+size_f_size));
                                   frame_ptr = src_frame;
                                   flag = complete;
                          }
                     }
                    package_ptr = p_dst;
              }
         }else{
                    p_info->off_size = offset;
                    return p_info;
         }
     }
     p_info->off_size = offset;
     return p_info;
}


int judge_package_all(int size,int offset,char *src){
      int all_size = 0;
      int flag = 1;
      unsigned char control;
      int size_f_size;
      int complete;
      int frame_size = 0;
      int i = 0;
      offset = offset + 10;
   if(size - offset > 0){
        while(flag){
            if(size - offset  > 2){
              control      = (unsigned char)(*(src + offset));
              size_f_size  = GET_LOW01_BITS(control) + 1;
              complete     = GET_LOW23_BITS(control);
              frame_size = 0;
              if(size > offset + 1 + size_f_size ){
                   for (i = 0; i < size_f_size; i++) {
                          frame_size += *(src + i + offset  + 1) << (8 * (size_f_size - 1 - i));
                   }
                   all_size = offset + size_f_size + frame_size +1;
                   printf("all_size = %d\n",all_size);
                   if(all_size > size){
                          return 0;
                   }else if(all_size == size){
                         if(complete == 0){
                                return 1;
                         }else{
                                return 0;
                         }
                  }else{
                         if(complete == 0){
                                return 1;
                         }else{
                                offset = offset + size_f_size + frame_size +1;
                         }
                  }
              }else{
                  return 0;  
              }
           }else{
                  return 0;
           }
       }
   }else{
         return 0;  
   }
}

void free_malloc(struct mfptp_parse * p)
{
       struct mfptp_parse * q = NULL;
       struct mfptp_frame * s = NULL;
       while(p){
             q = p->next; 
             while(p->m_frame){
                   s = p->m_frame->next;
                   if(p->m_frame->frame_buf != NULL){
                          free(p->m_frame->frame_buf);
                   }
                   free(p->m_frame);
                   p->m_frame = s; 
             }
             free(p);
             p = q;
       }
}
