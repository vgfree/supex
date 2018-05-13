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

/* MFPTP打包器的状态 */
struct mfptp_pack_stat
{
	char                    **pdata;	/* 已打包的数据缓冲区首地址 */
	size_t                  *psize;	/* 已打包的数据大小的地址 */
	size_t                  dosize;	/* 目前已打包数据的字节数 */
	enum mfptp_error        error;	/* MFPTP打包出错的信息 */
};

/* MFPTP打包器的相关信息 */
struct mfptp_packager
{
	bool                            init;		/* 判断此结构体是否被正确初始化 */
	struct mfptp_header_info        header;		/* MFPTP协议head相关信息 */
	struct mfptp_bodyer_info        bodyer;		/* MFPTP协议body相关信息 */
	struct mfptp_pack_stat          ms;		/* 打包器的状态 */
};

/***********************************************************************************
 * 功能：初始化打包器结构体
 * @pdata:保存打包好的数据缓冲区的地址  @psize：保存打包好的数据大小的地址
 ************************************************************************************/
void mfptp_package_init(struct mfptp_packager *packager, char **pdata, size_t *psize);

/***********************************************************************************
 * 功能：更正打包器结构体
 * @pdata:保存打包好的数据缓冲区的地址  @psize：保存打包好的数据大小的地址
 ************************************************************************************/
void mfptp_package_adjust(struct mfptp_packager *packager, char **pdata, size_t *psize);

/***********************************************************************************
 * 功能：销毁打包器结构体
 ************************************************************************************/
void mfptp_package_destroy(struct mfptp_packager *packager);

/***********************************************************************************
 * 功能：估算数据打包的内存大小
 * @frames:待打包数据的总帧数  @dsize:待打包数据的总大小
 * @返回值：>0 打包最大需要的内存大小
 ***********************************************************************************/
size_t mfptp_package_reckon(size_t frames, size_t dsize);

/***********************************************************************************
 * 功能：开始打包数据
 * @data:待打包的数据首地址
 * @flags:加密压缩的标志位
 * @stype:socket的type
 * @packages:待打包数据的总包数
 * @frame_offset:待打包数据的每帧偏移量
 * @frame_size:待打包数据的每帧大小
 * @frames_of_pack:待打包数据每个单包中帧的总数
 * @返回值: -1:失败, 否则代表打包之后的数据的大小
 ***********************************************************************************/
ssize_t mfptp_package(struct mfptp_packager *packager, const char *data,
		unsigned char flags, unsigned char stype,
		uint32_t serial_number,
		int packages,
		const int frame_offset[], const int frame_size[], const int frames_of_pack[]);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __MFPTP_PACKAGE_H__ */

