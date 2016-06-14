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

/* 解压和解密回调函数 */
typedef int (*Decompression_CallBack)(char *dest, const char *src, int d_len, int s_len);
typedef int (*Decryption_CallBack)(char *dest, const char *src, int d_len, int s_len);

/* MFPTP协议的状态码[当前解析或打包正在处理协议的哪部分] */
enum mfptp_status {
	MFPTP_PARSE_INIT = 0x00,/* MFPTP协议解析初始化的状态 */
	MFPTP_HEAD,		/* MFPTP协议的前6个字节"#MFPTP" */
	MFPTP_VERSION,		/* MFPTP协议的版本号 */
	MFPTP_CONFIG,		/* MFPTP协议的压缩解密格式 */
	MFPTP_SOCKET_TYPE,	/* MFPTP协议socket的类型 */
	MFPTP_PACKAGES,		/* MFPTP协议携带的包数 */
	MFPTP_FP_CONTROL,	/* MFPTP协议的FP_control字段 */
	MFPTP_F_SIZE,		/* MFPTP协议F_size字段 */
	MFPTP_FRAME_START,	/* MFPTP协议的帧 */
	MFPTP_FRAME_OVER,	/* MFPTP协议解析完一包的数据 */
	MFPTP_PARSE_OVER	/* MFPTP协议解析完*/
};

/* MFPTP解析器的状态 */
struct mfptp_parser_stat {
	bool                    over;		/* 是否完成解析*/
	bool			resume;		/* 是否断点续传 */
	int			dosize;		/* 当前已解析的长度 */
	char *const*            data;		/* 待解析数据起始地址指针*/
	int const*		dsize;		/* 待解析数据的总长度地址*/
	struct comm_cache	cache;		/* 保存解压解密之后的数据 */
	enum mfptp_status 	step;		/* 当前解析的步进 */
	enum mfptp_error	error;		/* MFPTP解析错误码 */
};

/* MFPTP解析器的相关信息 */
struct mfptp_parser {
	bool				init;		/* 此结构体是否被正确的初始化 */
	struct mfptp_header_info	header;		/* MFPTP协议head的相关信息 */
	struct mfptp_bodyer_info	bodyer;		/* MFPTP协议body的相关信息 */
	Decompression_CallBack		decompresscb;	/* 解压的回调函数 */
	Decryption_CallBack		decryptcb;	/* 解密的回调函数 */
	struct mfptp_parser_stat	ms;		/* MFPTP协议数据解析器状态 */
	char*				decryptbuff;	/* 保存解密之后的数据 */
	char*				decompressbuff;	/* 保存解压之后的数据 */
};

/***********************************************************************************
 * 功能：初始化解析结构体
 * @data:待解析数据缓冲区地址  @size:待解析数据大小的地址
***********************************************************************************/
bool mfptp_parse_init(struct mfptp_parser *parser, char* const *data, const int *size);     

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

#endif /* ifndef __MFPTP_PARSE_H__ */
