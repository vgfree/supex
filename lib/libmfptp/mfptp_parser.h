#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <openssl/rand.h>
#include "mfptp_utils.h"

#ifndef bool
  #define bool int
#endif

#ifndef true
  #define true 1
#endif

#ifndef false
  #define false                 0
#endif

#define MFPTP_PARSER_TEST       (0xff)

#define MFPTP_PARSE_NODATA      (-2)
#define MFPTP_PARSE_ILEGAL      (-1)
#define MFPTP_PARSE_INIT        0
#define MFPTP_PARSE_HEAD        1
#define MFPTP_PARSE_PACKAGE     2
#define MFPTP_PARSE_OVER        3
#define MFPTP_PARSE_MOREDATA    4
// #define MFPTP_PARSE_ERROR 5;

#define MFPTP_UID_MAX_LEN       (63)
#define MFPTP_INVALID_UID       "111111"
#define MFPTP_INVALID_UID_LEN   (6)
// #define MFPTP_INVALID_FD                (-1)

enum COMPRESSION_FORMAT_TYPE
{
	NO_COMPRESSION = 0x0,
	ZIP_COMPRESSION = 0x1,
	GZIP_COMPRESSION = 0x2,
};

enum ENCRYPTION_FORMAR_TYPE
{
	NO_ENCRYPTION = 0x0,
	IDEA_ENCRYPTION = 0x1,
	AES_ENCRYPTION = 0x2,
};

enum WORK_METHOD
{
	PAIR_METHOD = 0x00,
	PUB_METHOD = 0x01,
	SUB_METHOD = 0x02,
	REQ_METHOD = 0x03,
	REP_METHOD = 0x04,
	DEALER_METHOD = 0x05,
	ROUTER_METHOD = 0x06,
	PULL_METHOD = 0x07,
	PUSH_METHOD = 0x08,
	HEARTBEAT_METHOD = 0x09,
	INVALID_METHOD = 0x10
};

typedef void * (*mfptp_callback_func)(void *);
typedef int (*mfptp_send)(void *);
struct mfptp_parser
{
	short                   version;
	short                   sub_version;
	unsigned char           method;
	unsigned char           compress;
	unsigned char           encrypt;
	unsigned char           packages;		/*-package counts to parse-*/
	int                     pos_frame_start;	/*-data start pos-*/
	mfptp_callback_func     func;			/*-callback function-*/
};

/*- 整个数据可能含有几个包 -*/
/*- 每个包可能有几个帧 -*/

#define MAX_REQ_SIZE            10485760
#define MAX_FRAMES_PER_PACKAGE  13

struct mfptp_package
{
	bool    complete;
	int     frames;
	int     dsizes[MAX_FRAMES_PER_PACKAGE];	/*-data size, all size is :fsize + dsize + 1, 1 is control size-*/
	char    *ptrs[MAX_FRAMES_PER_PACKAGE];	/*-data start pointer-*/
};

struct mfptp_status
{
	int                     step;		/*-parse step-*/
	int                     index;		/*-parsing package index, start 1-*/
	int                     doptr;		/*-pointer now pos-*/
	int                     sharp_pos;	/* 记录每个包开始的#的位置, 主要是为了计算续传的位置*/
	struct mfptp_package    package;
};

struct mfptp_parser_info
{
	struct mfptp_parser     parser;
	struct mfptp_status     status;
	struct mfptp_key        m_key;
	int                     force_login;
	char                    who[64];
	/* char key[16]; */    /* 加密密钥，使用128bit长度*/
	char                    *buf;
	char                    *buf1;
	int                     out_len;
	int                     de_len;
	int                     snd_flg;
	mfptp_send              p_send;
};

int mfptp_send_callback(struct mfptp_parser_info *p_info, mfptp_send fun);

void mfptp_register_callback(struct mfptp_parser_info *p_info, mfptp_callback_func fun);

void mfptp_init_parser_info(struct mfptp_parser_info *p_info);

int mfptp_parse(uint8_t *buf_addr, int get_size, struct mfptp_parser_info *p_info);

void *mfptp_auth_callback(void *data);

void *mfptp_drift_out_callback(void *data);

// void mfptp_set_user_secret_key(struct mfptp_parser_info *p_info);

