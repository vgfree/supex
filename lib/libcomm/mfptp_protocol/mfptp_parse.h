/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/22.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __MFPTP_PARSE_H__
#define __MFPTP_PARSE_H__

#include "mfptp_utils.h"
#include "../comm_cache.h"

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
	MFPTP_PACKAGES,			/* 11 MFPTP协议携带的包数 */
	MFPTP_FP_CONTROL,		/* 12 MFPTP协议的FP_control字段 */
	MFPTP_F_SIZE_1,			/* 13 MFPTP协议F_size字段的第一位 */
	MFPTP_F_SIZE_2,			/* 14 MFPTP协议F_size字段的第二位 */
	MFPTP_F_SIZE_3,			/* 15 MFPTP协议F_size字段的第三位 */
	MFPTP_F_SIZE_4,			/* 16 MFPTP协议F_size字段的第四位 */
	MFPTP_FRAME_START,		/* 17 MFPTP协议的帧 */
	MFPTP_FRAME_SETTING,		/* 20 MFPTP协议设置帧的大小,偏移，帧数计数 */
	MFPTP_FRAME_OVER,		/* 21 MFPTP协议解析完一包的数据 */
	MFPTP_PARSE_OVER		/* 22 MFPTP协议解析完*/
};

/* MFPTP解析器的状态 */
struct mfptp_parser_stat
{
	bool                    over;		/* 是否完成解析*/
	bool                    resume;		/* 是否断点续传 */
	int                     dosize;		/* 当前已解析的长度 */
//	int			frame_offset;	/* 记录目前最后解析的帧偏移[已解析数据在cache里面的偏移]*/
	char *const             *data;		/* 待解析数据起始地址指针*/
	int const               *dsize;		/* 待解析数据的总长度地址*/
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

/***********************************************************************************
* 功能：初始化解析结构体
* @data:待解析数据缓冲区地址  @size:待解析数据大小的地址
***********************************************************************************/
void mfptp_parse_init(struct mfptp_parser *parser, char *const *data, const int *size);

/***********************************************************************************
* 功能：销毁一个解析结构体
***********************************************************************************/
void mfptp_parse_destroy(struct mfptp_parser *parser);

/***********************************************************************************
* 功能：开始解析数据
* 返回值：已解析数据的字节数
***********************************************************************************/
int mfptp_parse(struct mfptp_parser *parser);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __MFPTP_PARSE_H__ */

