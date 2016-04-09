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


     
struct comm_cache{
	int	start;		/* 有效数据的开始下标 */
	int	end;		/* 有效数据的结束下标 */
	int	size;		/* 有效数据的大小 */
	int	capacity;	/* 缓冲区的大小 */
	char*	cache;		/* 缓冲区的地址 */
};

/* 初始化cache */
bool commcache_init(struct comm_cache* comm_cache,  int capacity);

/* 往cache里面添加datasize大小的数据 */
bool commcache_append(struct comm_cache* comm_cache,  const char* data,  int datasize);

/* 减少cache有效数据的计数  @size:解析的数据的多少 */
void commcache_deccnt(struct comm_cache* comm_cache, int size);

/* 清除cache里面的无效数据 */
void commcache_clean(struct comm_cache* comm_cache);

/* 释放cache */
void commcache_free(struct comm_cahce* comm_cache);


     
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_CACHE_H__ */
