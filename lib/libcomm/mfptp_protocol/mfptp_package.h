/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/22.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __MFPTP_PACKAGE_H__
#define __MFPTP_PACKAGE_H__

#include "mfptp_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*Compression_CallBack)(char *dest, const char *src, int d_len, int s_len);
typedef int (*Encryption_CallBack)(char *dest, const char *src, int d_len, int s_len);


/* MFPTP打包器的状态 */
struct mfptp_pack_stat {
	char*			buff;	/* 用于存放打包好的数据的缓冲区 */
	int*			size;	/* 缓冲区已有数据大小 */
	int			dosize;	/* 目前打包的字节数 */
	enum mfptp_error	error;	/* MFPTP打包出错信息 */
};


/* MFPTP打包器的相关信息 */
struct mfptp_packager {
	bool				init;		/* 判断此结构体是否被正确初始化 */
	struct mfptp_header_info	header;		/* MFPTP协议头相关信息 */
	struct mfptp_package_info	package;	/* MFPTP协议包的具体信息 */
	Encryption_CallBack		encryptcb;	/* MFPTP协议加密的回调函数 */
	Compression_CallBack		compresscb;	/* MFPTP协议压缩的回调函数 */
	struct mfptp_pack_stat		ms;		/* 打包器的状态 */
};

/***********************************************************************************
 * 功能：初始化打包器结构体
 * @buff:用于存放打包好的数据的缓冲区  @size：缓冲区中已有的数据大小
 * @返回值：true:初始化成功 false：初始化失败
************************************************************************************/
void mfptp_package_init(struct mfptp_packager *packager, char *buff, int *size);

/***********************************************************************************
 * 功能：开始打包数据
 * @data:待打包的数据首地址 @flag:压缩和加密的标志位  @method:socket的type
 * @返回值: -1:失败, 否则代表打包之后的数据的大小
************************************************************************************/
int mfptp_package(struct mfptp_packager *packager, const char* data, unsigned char flag,  int method);

/***********************************************************************************
 * 功能：检测用于保存打包好的数据的内存大小是否足够 
 * @memsize:用于保存打包数据内存的大小 @packages:总包数
 * @frames:总帧数  @dsize:待打包数据的总大小
 * @返回值：>0代表空间不足还需的内存大小 <=0则代表空间足够
***********************************************************************************/
int mfptp_check_memory(int memsize, int packages, int frames, int dsize);

/***********************************************************************************
 * 功能：填充struct mfptp_packager_info结构体的数据
 * @packages:待打包的包数 @dsize:待打包数据的总大小
 * @frmoffset:待打包数据帧的偏移数组 @frames_of_pack:每包中的帧数
 * 返回值：总是成功
***********************************************************************************/
void mfptp_fill_package(struct mfptp_packager *packager, const int *frame_offset, const int *frame_size, const int *frames_of_pack, int packages);

#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __MFPTP_PACKAGE_H__ */
