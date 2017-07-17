/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/14.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_CACHE_H__
#define __COMM_CACHE_H__

#include "comm_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BASEBUFFERSIZE 1024	/*  CACHE里面buff的基本大小 */

struct comm_cache
{
	bool    init;			/* 结构体是否初始化的标志 */
	int     start;			/* 有效数据的开始下标 */
	int     end;			/* 有效数据的结束下标 */
	int     size;			/* 有效数据的大小 */
	int     capacity;		/* 缓冲区的大小 */
	char    base[BASEBUFFERSIZE];	/* 缓冲的基地址 */
	char    *buffer;		/* 缓冲区的地址 */
};

/* 初始化cache */
void commcache_init(struct comm_cache *comm_cache);

/* 释放cache */
void commcache_free(struct comm_cache *comm_cache);

/* 往cache里面添加数据 @data:待添加的数据 @datasize:待添加数据的大小 */
bool commcache_append(struct comm_cache *comm_cache, const char *data, int datasize);

/* 扩展cache的内存， @size:需要增加内存的大小，-1则使用默认值 */
bool commcache_expand(struct comm_cache *comm_cache, int size);

/* 恢复cache里面的buffer为基地址 */
void commcache_shrink(struct comm_cache *commcache);

/* 清除cache里面的无效数据 */
void commcache_adjust(struct comm_cache *comm_cache);


#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_CACHE_H__ */

