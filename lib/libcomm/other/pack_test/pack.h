#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
//#include "utils.h"
//#include "parse.h"


#define FRAME_MORE           100


struct mfptp_frame
{
   int   size;
   char  *frame_buf;
   struct mfptp_frame * next;
};

struct mfptp_pack
{
   short         version;
   short         sub_version;
   unsigned char compress;
   unsigned char encrypt;
   unsigned char method;
   unsigned char packages;
   char          *pack_buf;
 //  struct mfptp_key   m_key;    
   struct mfptp_frame *m_frame;
   struct mfptp_pack  *next;
};



int  mfptp_pack_len(int len);

struct mfptp_pack *copy_list(struct mfptp_pack *head,struct mfptp_pack *cop,int len);

struct mfptp_frame *copy_frame_list(struct mfptp_frame *head,struct mfptp_frame *cop,int len);

struct mfptp_pack * pack_one_package(struct mfptp_pack * p_info);

struct mfptp_pack *  mfptp_packages(struct mfptp_pack * p_info);
