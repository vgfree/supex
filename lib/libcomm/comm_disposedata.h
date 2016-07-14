/*********************************************************************************************/
/************************	Created by 许莉 on 16/07/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_DISPOSEDATA_H__
#define __COMM_DISPOSEDATA_H__

#include "comm_structure.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	ENCRYPTSIZE	1024	/* 加密缓冲区的默认大小 */
#define	COMPRESSIZE	1024	/* 压缩缓冲区的默认大小 */
#define	DECRYPTSIZE	1024	/* 解密缓冲区的默认大小 */
#define	DECOMPRESSIZE	1024	/* 解压缓冲区的默认大小 */


/* 处理数据[加密，压缩，解密，解压]的函数原型, 返回值为处理之后数据的大小 */
typedef int (*DisposeData_CallBack)(char *dest, const char *src, int d_len, int s_len);

bool  encrypt_compress_data(struct comm_cache *cache, struct comm_message *message);

bool  decrypt_decompress_data(struct comm_cache *cache, struct comm_message *message);



#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_DISPOSEDATA_H__ */
