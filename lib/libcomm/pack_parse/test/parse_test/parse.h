#pragma 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <openssl/rand.h>
//#include "utils.h"

#define MAX_REQ_SIZE           10485760

#ifndef bool
#define bool int
#endif
 

#ifndef true 
#define true 1
#endif


#ifndef false
#define false 0
#endif



enum WORK_METHOD{
         PAIR_METHOD      = 0x00,
         PUB_METHOD       = 0x01,
         SUB_METHOD       = 0x02,
         REQ_METHOD       = 0x03,
         REP_METHOD       = 0x04,
         DEALER_METHOD    = 0x05,
         ROUTER_METHOD    = 0x06,
         PULL_METHOD      = 0x07,
         PUSH_METHOD      = 0x08,
         HEARTBEAT_METHOD = 0x09,
         INVALID_METHOD   = 0x10
 };

enum COMPRESSION_FORMAT_TYPE{
         NO_COMPRESSION   =    0x0,
         ZIP_COMPRESSION  =    0x1,
         GZIP_COMPRESSION =    0x2,
};

enum ENCRYPTION_FORMAR_TYPE{
         NO_ENCRYPTION   =     0x0,
         IDEA_ENCRYPTION =     0x1,
         AES_ENCRYPTION  =     0x2,
 };


struct mfptp_frame
{
   int   size;
   char  *frame_buf;
   struct mfptp_frame * next;
};

struct mfptp_parse
{
   int    off_size;
   struct mfptp_frame *m_frame;
   struct mfptp_parse  *next;
};

int judge_package_all(int size,int offset,char *src);

void free_malloc(struct mfptp_parse * p);

struct mfptp_parse *  parse_data(int size,char *buf,struct mfptp_parse *p_info);
