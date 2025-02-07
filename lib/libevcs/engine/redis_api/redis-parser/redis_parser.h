//
//  redis_parser.h
//  supex
//
//  Created by 周凯 on 16/1/16.
//  Copyright © 2016年 zhoukai. All rights reserved.
//

#ifndef redis_parser_h
#define redis_parser_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#if defined(__cplusplus)
  #if !defined(__BEGIN_DECLS)
    #define __BEGIN_DECLS \
	extern "C" {
    #define __END_DECLS	\
	}
  #endif
#else
  #define __BEGIN_DECLS
  #define __END_DECLS
#endif

__BEGIN_DECLS

enum
{
	redis_reply_none,
	redis_reply_num,
	redis_reply_info,
	redis_reply_err,
	redis_reply_bulk,
	redis_reply_multi,
};

enum redis_parser_type
{
	REDIS_REQUEST = 0x01, REDIS_REPLY,
};

enum
{
	redis_command_none = 0,
	/* ---------- write ---------- */
	redis_command_set = 0x00003003,
	redis_command_del = 0x0000085f,
	redis_command_mset = 0x000367e3,
	redis_command_hset = 0x0002109b,
	redis_command_sadd = 0x0004d421,
	redis_command_lpush = 0x0050f033,
	redis_command_rpush = 0x007ac693,
	redis_command_lpushx = 0x08386545,
	redis_command_rpushx = 0x0c782b05,
	redis_command_publish = 4872020355,
	/* ---------- read ---------- */
	redis_command_get = 0x00001053,
	redis_command_hget = 0x0001f0eb,
	redis_command_hmget = 0x003417a3,
	redis_command_hgetall = 0x85448761,
	redis_command_lrange = 0x0840ece4,
	redis_command_keys = 0x0002bba2,
	redis_command_info = 0x00024824,
	redis_command_ping = 0x00041c50,
	redis_command_exists = 0x0377e5b8,

	/* ---------- option ---------- */
	redis_command_quit = 294963,
	redis_command_compact = 789918239,
	/* ---------- sync ---------- */
	redis_command_syncset = 1556657827,
	redis_command_syncdel = 1556647679,
	/* ---------- test ---------- */
	redis_command_values = 249715874,
};

typedef struct redis_parser redis_parser;
typedef struct redis_parser_settings redis_parser_settings;
typedef int (*redis_cb)(redis_parser *parser, int64_t value);
typedef int (*redis_data_cb)(redis_parser *parser, const char *data, size_t len);
struct redis_parser
{
	/*private : read only*/
	unsigned        type            : 6;	/**< 解析种类:REDIS_REQUEST/REDIS_REPLY*/
	unsigned        state           : 16;	/**< 当前解析状态，0为初始化状态， 0xffff为已解析完成*/
	unsigned        redis_errno     : 10;	/**< 发生的错误，0为没有错误*/
	int             fields;			/**< 还需要处理的字段数，在完成时为0或-1（表示空对象）*/
	struct
	{
		int64_t integer;
		int32_t negative        : 1;
		int32_t prec            : 7;
		int32_t fractional      : 24;
	}               parse_number;	/**< 当前如果在分析数字，则记录在此*/
	union
	{
		unsigned        reply_type;	/**< 回复类型*/
		uint64_t        command_type;	/**< 命令类型*/
	};
	int64_t         content_length;		/**< 当前字段内容的长度*/
	/*public : read and write*/
	void            *data;			/**<用户数据*/
};

/* ---------------------------------            *\
* 当回调函数返回非0值时，表示中止解析后续数据，	*
* 必须调用redis_parser_init()重新来过			*
\* ---------------------------------            */
struct redis_parser_settings
{
	/*在确定了响应类型或命令类型后回调，和将要解析的总字段数量*/
	redis_cb        on_message_begin;

	/*
	 *如果字段是以长度标示的，在获取了完整的长度后回调，
	 *如果长度为-1，则表示该字段为nil字段
	 * redis_reply_num,redis_reply_err,redis_reply_info类型的解析会在
	 * on_content()之后调用此函数
	 *在调用此函数时，在fields中给出剩余的字段数量
	 */
	redis_cb        on_content_len;

	/*
	 *解析普通响应消息或字段内容时回调，可能回调多次
	 *在调用此函数时，在fields中给出剩余的字段数量
	 */
	redis_data_cb   on_content;
	/*在解析完成后回调，第二个参数为0或-1，否则视代码有bug*/
	redis_cb        on_message_complete;
};

/**
 * 初始化解析器
 * 在每次解析一条新的协议数据前调用
 * @param type 必须是REDIS_REQUEST或REDIS_REPLY
 */
void redis_parser_init(redis_parser *parser, enum redis_parser_type type);

/**
 * 分析数据
 * 根据给出的数据可以多次调用，直到解析完成
 * @param settings 对应状态的回调
 * @param data 数据，可以为二进制
 * @param len 数据的长度
 * @return 本次解析的长度为0，且传入数据长度不为0，则可能已经解析到完整的协议数据，
 * 或发生了错误（会设置parser.redis_errno字段为相应的错误值）
 */
size_t redis_parser_execute(redis_parser *parser, struct redis_parser_settings *settings, const char *data, size_t len);

/**
 * 根据错误值获取错误名称
 */
const char *redis_errno_name(int redis_errno);

/**
 * 根据错误值获取错误描述，符合redis错误响应字串
 */
const char *redis_errno_description(int redis_errno);

/**
 * 得到命令名
 */
const char *redis_command_name(uint64_t command);

__END_DECLS
#endif	/* redis_parser_h */

