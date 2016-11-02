#pragma once
#include "x_utils.h"
#include "basic_type.h"

#include <ev.h>
#include <arpa/inet.h>

#include "list.h"
#include "net_cache.h"
#include "rbtree.h"
#include "mfptp_parser.h"
#include "mfptp_api.h"

#define MFPTP_UID_MAX_LEN       (63)
#define MFPTP_INVALID_UID       "111111"
#define MFPTP_INVALID_UID_LEN   (6)
#define MFPTP_INVALID_FD        (-1)

#define MAX_CHANNEL_NAME_LEN    256
#pragma pack(1)
// 用户操作数据结构
typedef struct
{
	unsigned char   type;				// 操作类型'1'是用户加入某个频道 '2'是用户脱离某个频道
	char            IMEI[15];			// 用户的IMEI
	char            channel[MAX_CHANNEL_NAME_LEN];	// 要加入或删除的频道
} user_oper;
#pragma pack()

#define USER_ADD_CHANNEL        ('1')
#define USER_DELETE_CHANNEL     ('2')
#define USER_ILLEGAL_OPER       ('0')
#define OPER_LEN                (1)		//传递的消息中操作字段占的字节数
#define IMEI_LEN                (15)		// ＩＭＥＩ的长度

