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
	char                    **buff;	/* 已打包的数据缓冲区首地址 */
	int                     *size;	/* 已打包的数据大小的地址 */
	int                     dosize;	/* 目前已打包数据的字节数 */
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
 * @buff:保存打包好的数据缓冲区的地址  @size：保存打包好的数据大小的地址
 ************************************************************************************/
void mfptp_package_init(struct mfptp_packager *packager, char **buff, int *size);

/***********************************************************************************
 * 功能：销毁打包器结构体
 ************************************************************************************/
void mfptp_package_destroy(struct mfptp_packager *packager);

/***********************************************************************************
 * 功能：开始打包数据
 * @data:待打包的数据首地址  @method:socket的type
 * @返回值: -1:失败, 否则代表打包之后的数据的大小
 ************************************************************************************/
int mfptp_package(struct mfptp_packager *packager, const char *data, int method);

/***********************************************************************************
* 功能：检测保存已打包数据的内存大小是否足够
* @memsize:保存打包数据内存的大小
* @frames:待打包数据的总帧数  @dsize:待打包数据的总大小
* @返回值：>0代表空间不足，且还需的内存大小 <=0则代表空间足够
***********************************************************************************/
int mfptp_check_memory(int memsize, int frames, int dsize);

/***********************************************************************************
* 功能：填充struct mfptp_packager_info结构体的数据
* @frame_offset:待打包数据的每帧偏移量
* @frame_size:待打包数据的每帧大小
* @frames_of_pack:待打包数据每个单包中帧的总数
* @packages:待打包数据的总包数
***********************************************************************************/
void mfptp_fill_package(struct mfptp_packager *packager, const int *frame_offset, const int *frame_size, const int *frames_of_pack, int packages);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __MFPTP_PACKAGE_H__ */

