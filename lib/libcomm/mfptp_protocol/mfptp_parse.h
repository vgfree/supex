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



/* MFPTP解析器的状态 */
struct mfptp_parser_stat {
	bool                    over;		/* 是否完成解析*/
	bool			resume;		/* 是否断点续传 */
	int			step;		/* 当前解析的步进 */
	int			dosize;		/* 当前已解析的长度 */
	char *const*            data;		/* 被解析数据起始地址指针*/
	int const*		size;		/* 当前数据的总长度地址*/
	enum mfptp_error	error;		/* MFPTP解析错误码 */
};

/* MFPTP解析器[解析之后相关数据信息] */
struct mfptp_parser {
	struct mfptp_header_info	header;	/* MFPTP头的相关信息 */
	struct mfptp_package_info	package;/* 每个包的相关信息 */
};

/* MFPTP解析器的相关信息 */
struct mfptp_parser_info {
	struct mfptp_parser		mp;	/* MFPTP数据解析器 */
	struct mfptp_parser_stat	ms;	/* MFPTP数据解析器状态 */
};

/***********************************************************************************
 * 功能：初始化解析结构体
 * @data:待解析数据缓冲区地址  @size:待解析数据大小的地址
 * 返回值：true:初始化成功 false:初始化失败
***********************************************************************************/
bool mfptp_parse_init(struct mfptp_parser_info *parser, char* const* data, const int *size);     

/***********************************************************************************
 * 功能：开始解析数据
 * 返回值：已解析数据的字节数
***********************************************************************************/
int mfptp_parse(struct mfptp_parser_info* parser);
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __MFPTP_PARSE_H__ */
