/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/22.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __MFPTP_PARSE_H__
#define __MFPTP_PARSE_H__

#include "mfptp_utils.h"

#ifdef __cplusplus
extern "C" {
#endif


/* MFPTP协议的状态码[当前解析或打包正在处理协议的哪部分] */
enum mfptp_status
{
	MFPTP_PARSE_INIT = 0x00,	/*  0 MFPTP协议解析初始化的状态 */
	MFPTP_HEAD_FIRST,		/*  1 MFPTP协议的第一个字节'#' */
	MFPTP_HEAD_M,			/*  3 MFPTP协议的第二个字节'M'*/
	MFPTP_HEAD_F,			/*  4 MFPTP协议的第三个字节'F'*/
	MFPTP_HEAD_P,			/*  5 MFPTP协议的第四个字节'P'*/
	MFPTP_HEAD_T,			/*  6 MFPTP协议的第五个字节'T'*/
	MFPTP_HEAD_LAST,		/*  7 MFPTP协议的最后一个字节'P'*/
	MFPTP_VERSION,			/*  8 MFPTP协议的版本号 */
	MFPTP_CONFIG,			/*  9 MFPTP协议的压缩解密格式 */
	MFPTP_SOCKET_TYPE,		/* 10 MFPTP协议socket的类型 */
	MFPTP_SERIAL_NO_1,		/* 11 MFPTP协议serial_number字段的第一字节 */
	MFPTP_SERIAL_NO_2,		/* 12 MFPTP协议serial_number字段的第二字节 */
	MFPTP_SERIAL_NO_3,		/* 13 MFPTP协议serial_number字段的第三字节 */
	MFPTP_SERIAL_NO_4,		/* 14 MFPTP协议serial_number字段的第四字节 */
	MFPTP_PACKAGES,			/* 15 MFPTP协议携带的包数 */

	MFPTP_FP_CONTROL,		/* 16 MFPTP协议的FP_control字段 */
	MFPTP_F_SIZE_1,			/* 17 MFPTP协议F_size字段的第一字节 */
	MFPTP_F_SIZE_2,			/* 18 MFPTP协议F_size字段的第二字节 */
	MFPTP_F_SIZE_3,			/* 19 MFPTP协议F_size字段的第三字节 */
	MFPTP_F_SIZE_4,			/* 20 MFPTP协议F_size字段的第四字节 */
	MFPTP_FRAME_START,		/* 21 MFPTP协议的帧 */
	MFPTP_FRAME_OVER,		/* 22 MFPTP协议设置帧的大小,偏移，帧数计数 */
	MFPTP_PACKAGE_OVER,		/* 23 MFPTP协议解析完一包的数据 */
	MFPTP_PARSE_OVER		/* 24 MFPTP协议解析完*/
};

/* MFPTP解析器的状态 */
struct mfptp_parser_stat
{
	bool                    over;		/* 是否完成解析*/
	size_t                  dosize;		/* 当前已解析的长度 */
	char *const             *pdata;		/* 待解析数据起始地址指针*/
	size_t const            *psize;		/* 待解析数据的总长度地址*/
	enum mfptp_status       step;		/* 当前解析的步进 */
	enum mfptp_error        error;		/* MFPTP解析错误码 */
};

/* MFPTP解析器的相关信息 */
struct mfptp_parser
{
	bool                            init;			/* 此结构体是否被正确的初始化 */
	struct mfptp_header_info        header;			/* MFPTP协议head的相关信息 */
	struct mfptp_bodyer_info        bodyer;			/* MFPTP协议body的相关信息 */
	struct mfptp_parser_stat        ms;			/* MFPTP协议数据解析器状态 */
};

#define mfptp_parse_info	mfptp_parser

/***********************************************************************************
* 功能：初始化解析结构体
* @pdata:待解析数据缓冲区地址  @psize:待解析数据大小的地址
***********************************************************************************/
void mfptp_parse_init(struct mfptp_parser *parser, char *const *pdata, size_t const *psize);

/***********************************************************************************
* 功能：更正解析结构体
* @pdata:待解析数据缓冲区地址  @psize:待解析数据大小的地址
***********************************************************************************/
void mfptp_parse_adjust(struct mfptp_parser *parser, char *const *pdata, size_t const *psize);

/***********************************************************************************
* 功能：销毁一个解析结构体
***********************************************************************************/
void mfptp_parse_destroy(struct mfptp_parser *parser);

/***********************************************************************************
* 功能：开始解析数据
* 返回值：数据协议解析完成的字节数
* 	= 0代表解析未完成
* 	> 0代表解析已完成
* 		error == MFPTP_OK代表解析一条协议
* 		error != MFPTP_OK代表协议解析出错
***********************************************************************************/
ssize_t mfptp_parse(struct mfptp_parser *parser);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __MFPTP_PARSE_H__ */

